#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "uart_log.h"

#define LOG_UART_BAUD        115200U
#define LOG_QUEUE_LENGTH    32U
#define LOG_STACK_WORDS     384U
#define LOG_TASK_PRIORITY   1U

typedef struct {
    uart_log_event_t event;
    TickType_t tick;
} log_record_t;

static UART_HandleTypeDef s_uart;
static QueueHandle_t s_log_queue;

/* Watch these in the debugger; overflow must not stall the timer daemon. */
volatile uint32_t g_uart_log_dropped;
volatile uint32_t g_uart_log_errors;
volatile uint32_t g_uart_log_sent;

static const char *event_name(uart_log_event_t event)
{
    switch (event) {
    case UART_LOG_PRESS:   return "PRESS";
    case UART_LOG_RELEASE: return "RELEASE";
    case UART_LOG_HELD:    return "HELD";
    default:              return "UNKNOWN";
    }
}

static void transmit_line(char *line, int length, size_t capacity)
{
    if (length <= 0 || (size_t)length >= capacity) {
        g_uart_log_errors++;
        return;
    }

    /* Polling UART is confined to this task, never a timer callback or ISR. */
    if (HAL_UART_Transmit(&s_uart, (uint8_t *)line,
                          (uint16_t)length, 100U) != HAL_OK) {
        g_uart_log_errors++;
        return;
    }
    g_uart_log_sent++;
}

static void uart_log_task(void *argument)
{
    (void)argument;
    char line[128];

    int length = snprintf(
        line, sizeof(line),
        "READY baud=%lu tick_hz=%lu debounce_ms=%lu longpress_ms=%lu\r\n",
        (unsigned long)LOG_UART_BAUD,
        (unsigned long)configTICK_RATE_HZ,
        (unsigned long)DEBOUNCE_MS,
        (unsigned long)LONGPRESS_MS
    );
    transmit_line(line, length, sizeof(line));

    for (;;) {
        log_record_t record;
        if (xQueueReceive(s_log_queue, &record, portMAX_DELAY) != pdPASS) {
            continue;
        }

        unsigned long ms = (unsigned long)(
            ((uint64_t)record.tick * 1000U) / configTICK_RATE_HZ
        );
        length = snprintf(
            line, sizeof(line), "%s tick=%lu ms=%lu\r\n",
            event_name(record.event), (unsigned long)record.tick, ms
        );
        transmit_line(line, length, sizeof(line));
    }
}

void uart_log_init(void)
{   
    // Khoi tao gpio
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    // Khoi tao uart
    s_uart.Instance = USART1;
    s_uart.Init.BaudRate = LOG_UART_BAUD;
    s_uart.Init.WordLength = UART_WORDLENGTH_8B;
    s_uart.Init.StopBits = UART_STOPBITS_1;
    s_uart.Init.Parity = UART_PARITY_NONE;
    s_uart.Init.Mode = UART_MODE_TX;
    s_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_StatusTypeDef uart_status = HAL_UART_Init(&s_uart);
    configASSERT(uart_status == HAL_OK);

    // Khoi tao queue de truyen record vao  
    s_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_record_t));
    configASSERT(s_log_queue != NULL);

    BaseType_t task_status = xTaskCreate(
        uart_log_task, "uart_log", LOG_STACK_WORDS,
        NULL, LOG_TASK_PRIORITY, NULL
    );
    configASSERT(task_status == pdPASS);
}

// Ham gui vao log event
BaseType_t uart_log_event(uart_log_event_t event, TickType_t tick)
{
    configASSERT(s_log_queue != NULL);
    const log_record_t record = { .event = event, .tick = tick };
    BaseType_t status = xQueueSend(s_log_queue, &record, 0);
    if (status != pdPASS) {
        g_uart_log_dropped++;
    }
    return status;
}
