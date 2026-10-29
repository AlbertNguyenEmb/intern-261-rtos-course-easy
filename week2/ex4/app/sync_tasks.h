#ifndef SYNC_TASKS_H
#define SYNC_TASKS_H

#include <stdint.h>

#ifndef PRODUCER0_USE_NOTIFY
#define PRODUCER0_USE_NOTIFY 0
#endif

void sync_init(void);
void producer_task(void *argument);
void sync_task(void *argument);

extern void *sync_task_handle;

/* Variables for debugger, UART log, and REPORT.md. */
extern volatile uint32_t g_event_group_object_bytes;
extern volatile uint32_t g_event_group_heap_bytes;
extern volatile uint32_t g_sync_count;
extern volatile uint32_t g_timeout_count;
extern volatile uint32_t g_last_sync_period_ticks;
extern volatile uint32_t g_last_wakeup_cycles;
extern volatile uint32_t g_min_wakeup_cycles;
extern volatile uint32_t g_max_wakeup_cycles;
extern volatile uint32_t g_average_wakeup_cycles;

#endif /* SYNC_TASKS_H */
