/* Week 1 / Ex3 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

void periodic_task(void *argument);
void dwt_init(void);

#ifndef OVERHEAD_TEST
#define OVERHEAD_TEST 0
#endif

#define OVERHEAD_SAMPLE_COUNT 10U

/* Read these variables from the debugger Watch window. */
volatile uint32_t g_idle_loop_count;
volatile uint32_t g_loops_per_second;
volatile uint32_t g_overhead_samples[OVERHEAD_SAMPLE_COUNT];
volatile uint32_t g_overhead_sample_count;

static void counter_task(void *argument)
{
    (void)argument;

    for (;;) {
        g_idle_loop_count++;
    }
}

static void monitor_task(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();

    (void)argument;

    for (;;) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000));

        g_loops_per_second = g_idle_loop_count;
        g_idle_loop_count = 0;

        if (g_overhead_sample_count < OVERHEAD_SAMPLE_COUNT) {
            g_overhead_samples[g_overhead_sample_count] = g_loops_per_second;
            g_overhead_sample_count++;
        }
    }
}

void app_main(void)
{
#if OVERHEAD_TEST == 1
    /* The counter is the lowest-priority application task. The monitor wakes
     * once per second, snapshots the counter, then blocks again. Do not create
     * periodic_task in this mode: its workload would contaminate the tick ISR
     * overhead comparison. */
    BaseType_t result = xTaskCreate(counter_task, "Counter",
                                    configMINIMAL_STACK_SIZE, NULL,
                                    tskIDLE_PRIORITY + 1U, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(monitor_task, "Monitor",
                         configMINIMAL_STACK_SIZE, NULL,
                         tskIDLE_PRIORITY + 2U, NULL);
    configASSERT(result == pdPASS);
#else
    dwt_init();

    BaseType_t result = xTaskCreate(periodic_task, "Periodic", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1U, NULL);
    configASSERT(result == pdPASS);
#endif

    vTaskStartScheduler();

    for (;;) {
    }
}
