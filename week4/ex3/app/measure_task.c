/* Week 4 / Ex3 -- CPU%/stack HWM measurement + worst-case load generator.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "measure_task.h"

void measure_timer_init(void)
{
    /* TODO 1: identical DWT enable sequence as week3/ex4
     *         runtime_stats_timer_init() -- if that task already exists in
     *         this project, call it directly instead of duplicating. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t measure_timer_value(void)
{
    /* TODO 2: return DWT->CYCCNT, same as week3/ex4. */
    return DWT->CYCCNT;
}

void measure_task(void *argument)
{
    (void)argument;
    static char buf[512];

    for (;;) {
        /* TODO 3: vTaskGetRunTimeStats(buf) -> print over UART.
         *
         * TODO 4: for every task handle you created in week4/ex2/app_main.c,
         *         print uxTaskGetStackHighWaterMark(handle) * sizeof(StackType_t)
         *         in bytes -- that is the margin left, not the stack used. */
        vTaskDelay(pdMS_TO_TICKS(STATS_WINDOW_MS));
    }
}

void noise_task(void *argument)
{
    (void)argument;
    for (;;) {
        /* TODO 5: tight busy-loop (no vTaskDelay) sized to steal enough CPU
         *         that logger_task gets measurably delayed -- this is what
         *         makes the latency measurement in README step 3 an actual
         *         worst-case instead of an idle-system number. Document the
         *         loop size/duration you picked in REPORT.md. */
    }
}
