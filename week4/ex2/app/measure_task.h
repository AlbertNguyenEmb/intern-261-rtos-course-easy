#ifndef MEASURE_TASK_H
#define MEASURE_TASK_H

#include <stdint.h>

/* Same DWT CYCCNT time base pattern as week3/ex4 runtime_stats.c -- reuse
 * it, do not invent a second mechanism. */
void measure_timer_init(void);
uint32_t measure_timer_value(void);

/* Prints vTaskGetRunTimeStats() (CPU%) and uxTaskGetStackHighWaterMark()
 * for every task to UART every STATS_WINDOW_MS. */
void measure_task(void *argument);

/* Busy-loop "noise" task at a priority ABOVE logger_task, used only to
 * force the worst-case load described in README step 2. Not part of the
 * deliverable logger -- gate it so it is easy to disable for the normal
 * (non-worst-case) run. */
void noise_task(void *argument);

#endif /* MEASURE_TASK_H */
