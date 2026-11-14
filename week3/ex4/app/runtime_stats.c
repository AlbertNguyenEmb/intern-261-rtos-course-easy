/* Week 3 / Ex4 -- run-time stats on DWT CYCCNT.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include <string.h>
#include "deadlock_demo.h"
#include "supervisor.h"
static UART_HandleTypeDef s_stats_uart;

static void stats_uart_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    s_stats_uart.Instance = USART1;
    s_stats_uart.Init.BaudRate = 115200;
    s_stats_uart.Init.WordLength = UART_WORDLENGTH_8B;
    s_stats_uart.Init.StopBits = UART_STOPBITS_1;
    s_stats_uart.Init.Parity = UART_PARITY_NONE;
    s_stats_uart.Init.Mode = UART_MODE_TX;
    s_stats_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    s_stats_uart.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef status = HAL_UART_Init(&s_stats_uart);
    configASSERT(status == HAL_OK);
}

static void stats_uart_write(const char *text)
{
    size_t length = strlen(text);

    if (length == 0U) {
        return;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(
        &s_stats_uart,
        (uint8_t *)text,
        (uint16_t)length,
        100U
    );

    configASSERT(status == HAL_OK);
}

static const char *state_name(eTaskState state)
{
    switch (state) {
    case eRunning:   return "eRunning";
    case eReady:     return "eReady";
    case eBlocked:   return "eBlocked";
    case eSuspended: return "eSuspended";
    case eDeleted:   return "eDeleted";
    default:         return "unknown";
    }
}

static void print_task_states(void)
{
    static TaskStatus_t tasks[8];
    char line[128];

    TickType_t stamp = xTaskGetTickCount();
    UBaseType_t count = uxTaskGetSystemState(tasks, 8, NULL);

    if (count == 0U) {
        stats_uart_write("STATE: snapshot failed\r\n");
        return;
    }

    snprintf(line, sizeof(line),
             "\r\nSTATE tick=%lu\r\nTask\tState\tPriority\r\n",
             (unsigned long)stamp);
    stats_uart_write(line);

    for (UBaseType_t i = 0; i < count; i++) {
        snprintf(line, sizeof(line), "%s\t%s\t%lu\r\n",
                 tasks[i].pcTaskName,
                 state_name(tasks[i].eCurrentState),
                 (unsigned long)tasks[i].uxCurrentPriority);

        stats_uart_write(line);
    }
    snprintf(line, sizeof(line),
         "PROGRESS alpha=%lu beta=%lu\r\n",
         g_alpha_cycles, g_beta_cycles);
    stats_uart_write(line);

    snprintf(line, sizeof(line),
             "WDG feeds=%lu misses=%lu last_feed_tick=%lu\r\n",
             (unsigned long)g_wdg_feed_count,
             (unsigned long)g_wdg_miss_count,
             (unsigned long)g_wdg_last_feed_tick);
    stats_uart_write(line);
}

void runtime_stats_timer_init(void)
{
    /* TODO 1: same DWT enable sequence as week1/ex2 dwt_init() (CoreDebug
     *         DEMCR TRCENA bit, zero DWT->CYCCNT, DWT->CTRL CYCCNTENA bit).
     *         If you already have week1/ex2's dwt_init(), the sequence is
     *         identical -- do not invent a second mechanism. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t runtime_stats_timer_value(void)
{
    /* TODO 2: return DWT->CYCCNT. This is portGET_RUN_TIME_COUNTER_VALUE()'s
     *         implementation -- wire it up in FreeRTOSConfig.deltas. */
    return DWT->CYCCNT;
}

void runtime_stats_task(void *argument)
{
    (void)argument;
    static char buf[512];
    char heading[96];

    stats_uart_init();
    stats_uart_write("READY: CPU runtime statistics\r\n");

    snprintf(heading, sizeof(heading),
             "BOOT iwdg_reset=%lu\r\n",
             (unsigned long)g_boot_was_iwdg);
    stats_uart_write(heading);

    for (;;) {
        /* TODO 3: vTaskGetRunTimeStats(buf) and print buf over UART.
         *         Note: this walks every task's TCB -- it is itself not free,
         *         factor that into why STATS_WINDOW_MS should not be too
         *         short. */
        vTaskDelay(pdMS_TO_TICKS(STATS_WINDOW_MS));
        vTaskGetRunTimeStats(buf);

        int length = snprintf(heading, sizeof(heading), "\r\nCPU tick=%lu\r\n"
            "Task\tCycles\tCPU%%\r\n",
            (unsigned long)xTaskGetTickCount());

        if (length > 0 && (size_t)length < sizeof(heading)) {
            stats_uart_write(heading);
        }

        stats_uart_write(buf);
        print_task_states();
        supervisor_checkin(SUPERVISOR_BIT_STATS);
    }
}
