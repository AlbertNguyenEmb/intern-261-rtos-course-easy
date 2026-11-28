/* Week 4 / Ex3 -- CPU%/stack HWM measurement + worst-case load generator.
 * Native FreeRTOS API only.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "measure_task.h"

#ifndef STATS_WINDOW_MS
#define STATS_WINDOW_MS  5000   /* print stats every 5 seconds */
#endif

extern UART_HandleTypeDef huart1;
extern TaskHandle_t xLoggerHandle;
extern TaskHandle_t xSamplerHandle;
extern TaskHandle_t xProcessHandle;
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

        /* TODO 3: print CPU% */
        vTaskGetRunTimeStats(buf);
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)strlen(buf), HAL_MAX_DELAY);

        /* TODO 4: print stack HWM for every task */
        UBaseType_t mSampler = uxTaskGetStackHighWaterMark(xSamplerHandle) * sizeof(StackType_t);
        UBaseType_t mProcess = uxTaskGetStackHighWaterMark(xProcessHandle) * sizeof(StackType_t);
        UBaseType_t mLogger  = uxTaskGetStackHighWaterMark(xLoggerHandle)  * sizeof(StackType_t);
        UBaseType_t mStats   = uxTaskGetStackHighWaterMark(NULL)           * sizeof(StackType_t);

        static char msg[256];
        int len = snprintf(msg, sizeof(msg),
                           "Stack margin (bytes) -> Sampler: %lu | Process: %lu | Logger: %lu | Stats: %lu\r\n",
                           (unsigned long)mSampler,
                           (unsigned long)mProcess,
                           (unsigned long)mLogger,
                           (unsigned long)mStats);
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)len, HAL_MAX_DELAY);
    }
}

void noise_task(void *argument)
{
    (void)argument;
    for (;;) {
        for (volatile uint32_t i = 0; i < 100000; i++) {
            __NOP();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
