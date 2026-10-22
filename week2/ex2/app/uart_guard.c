/* Week 2 / Ex2 — four ways to serialize access to USART1 from WRITER_COUNT
 * tasks. Only one UART_GUARD_MODE is compiled at a time.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "uart_guard.h"

#define UART_BAUD        115200U
#define UART_TIMEOUT_MS  100U
#define GATEKEEPER_STACK_WORDS 256U
#define GATEKEEPER_QUEUE_LENGTH WRITER_COUNT

static UART_HandleTypeDef s_huart1;

#if UART_GUARD_MODE == 2
volatile uint32_t g_critical_last_us;
volatile uint32_t g_critical_avg_us;
volatile uint32_t g_critical_max_us;
volatile uint32_t g_critical_count;
volatile uint64_t g_critical_sum_us;
#endif

#if UART_GUARD_MODE == 1
static SemaphoreHandle_t s_uart_mutex;
#elif UART_GUARD_MODE == 3
static QueueHandle_t s_gatekeeper_queue;
static TaskHandle_t s_writer_handles[WRITER_COUNT];
#endif

static void uart1_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    s_huart1.Instance = USART1;
    s_huart1.Init.BaudRate = UART_BAUD;
    s_huart1.Init.WordLength = UART_WORDLENGTH_8B;
    s_huart1.Init.StopBits = UART_STOPBITS_1;
    s_huart1.Init.Parity = UART_PARITY_NONE;
    s_huart1.Init.Mode = UART_MODE_TX;
    s_huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    configASSERT(HAL_UART_Init(&s_huart1) == HAL_OK);
}
// send each byte uart
static void uart_write_bytes(const char *buf, size_t len)
{
    for (size_t i = 0U; i < len; i++) {
        HAL_StatusTypeDef status;

        do {
            status = HAL_UART_Transmit(
                &s_huart1,
                (uint8_t *)&buf[i],
                1U,
                UART_TIMEOUT_MS
            );

            if (status == HAL_BUSY) {
                taskYIELD();
            }
        } while (status == HAL_BUSY);
    }
}

void uart_guard_init(void)
{
    uart1_init();
#if UART_GUARD_MODE == 1
    /* TODO 1: xSemaphoreCreateMutex(). Check for NULL. */
    s_uart_mutex = xSemaphoreCreateMutex();
    configASSERT(s_uart_mutex != NULL);
#elif UART_GUARD_MODE == 3
    /* TODO 2: create s_gatekeeper_queue (length: pick and justify in
     *         REPORT.md), item size = MSG_LEN bytes.
     *         xTaskCreate(gatekeeper_task, ..., GATEKEEPER_PRIO). */
    BaseType_t status;

    s_gatekeeper_queue = xQueueCreate(
        GATEKEEPER_QUEUE_LENGTH,
        MSG_LEN
    );
    configASSERT(s_gatekeeper_queue != NULL);

    status = xTaskCreate(
        gatekeeper_task,
        "uart_gate",
        GATEKEEPER_STACK_WORDS,
        NULL,
        GATEKEEPER_PRIO,
        NULL
    );
    configASSERT(status == pdPASS);
#endif
}

void uart_guard_send(const char *buf, size_t len)
{
#if UART_GUARD_MODE == 0
    /* The bug: no serialization at all. */
    /* TODO 3: HAL_UART_Transmit directly. */
    uart_write_bytes(buf, len);

#elif UART_GUARD_MODE == 1
    /* TODO 4: xSemaphoreTake(s_uart_mutex, portMAX_DELAY), transmit, Give.
     *         Every return path must Give — no early return between Take and
     *         Give. */
    (void)buf; (void)len;
    if (xSemaphoreTake(s_uart_mutex, portMAX_DELAY) == pdTRUE) {
        uart_write_bytes(buf, len);
        configASSERT(
            xSemaphoreGive(s_uart_mutex) == pdTRUE
        );
    }
#elif UART_GUARD_MODE == 2
    /* TODO 5: taskENTER_CRITICAL(), transmit, taskEXIT_CRITICAL().
     *         Measure in REPORT.md how long the critical section holds IRQs
     *         off, as a function of MSG_LEN and UART_BAUD. */
    uint32_t start_cycles;
    uint32_t elapsed_us;

    taskENTER_CRITICAL();
    start_cycles = DWT->CYCCNT;
    uart_write_bytes(buf, len);

    elapsed_us = (DWT->CYCCNT - start_cycles) /
                 (SystemCoreClock / 1000000U);

    g_critical_last_us = elapsed_us;
    g_critical_sum_us += elapsed_us;
    g_critical_count++;
    g_critical_avg_us = (uint32_t)(
        g_critical_sum_us / g_critical_count
    );

    if (elapsed_us > g_critical_max_us) {
        g_critical_max_us = elapsed_us;
    }

    taskEXIT_CRITICAL();

#elif UART_GUARD_MODE == 3
    /* TODO 6: xQueueSend(s_gatekeeper_queue, buf, portMAX_DELAY).
     *         The writer never touches UART directly in this mode. */
    uint32_t writer_id;

    configASSERT(len == MSG_LEN);
    writer_id = (uint32_t)(buf[2] - '0');
    configASSERT(writer_id < WRITER_COUNT);

    /* Register before enqueue: the higher-priority gatekeeper can preempt the
     * writer as soon as xQueueSend() makes it Ready. */
    s_writer_handles[writer_id] = xTaskGetCurrentTaskHandle();

    configASSERT(
        xQueueSend(
            s_gatekeeper_queue,
            buf,
            portMAX_DELAY
        ) == pdPASS
    );

    /* Measure through actual UART completion, not just queue insertion. */
    configASSERT(
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0U
    );
#endif
}

#if UART_GUARD_MODE == 3
void gatekeeper_task(void *argument)
{
    (void)argument;
    for (;;) {
        char msg[MSG_LEN];
        uint32_t writer_id;
        /* TODO 7: xQueueReceive(s_gatekeeper_queue, msg, portMAX_DELAY), then
         *         HAL_UART_Transmit. This is the only task allowed to call
         *         HAL_UART_Transmit in this mode. */
        configASSERT(
            xQueueReceive(
                s_gatekeeper_queue,
                msg,
                portMAX_DELAY
            ) == pdPASS
        );

        uart_write_bytes(msg, MSG_LEN);

        writer_id = (uint32_t)(msg[2] - '0');
        configASSERT(writer_id < WRITER_COUNT);
        configASSERT(s_writer_handles[writer_id] != NULL);

        xTaskNotifyGive(s_writer_handles[writer_id]);
    }
}
#endif
