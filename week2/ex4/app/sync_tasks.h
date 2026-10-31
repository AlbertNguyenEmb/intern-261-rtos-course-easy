#ifndef SYNC_TASKS_H
#define SYNC_TASKS_H

#include <stdint.h>

#ifndef PRODUCER0_USE_NOTIFY
#define PRODUCER0_USE_NOTIFY 0
#endif

#ifndef EX4_FORCE_TIMEOUT
#define EX4_FORCE_TIMEOUT 0
#endif

#ifndef EX4_EXTRA_PRODUCER
#define EX4_EXTRA_PRODUCER 0
#endif

#define EX4_ACTIVE_PRODUCER_COUNT \
    (EVENT_BIT_COUNT + EX4_EXTRA_PRODUCER)

void sync_init(void);
void producer_task(void *argument);
void sync_task(void *argument);
void ram_debug_measurement_ready(void);
void ram_debug_tasks_created(void);

extern void *sync_task_handle;

/* Variables for debugger, UART log, and REPORT.md. */
extern volatile uint32_t g_event_group_object_bytes;
extern volatile uint32_t g_event_group_heap_bytes;
extern volatile uint32_t g_heap_before_bytes;
extern volatile uint32_t g_heap_after_bytes;
extern volatile uint32_t g_heap_after_tasks_bytes;
extern volatile uint32_t g_total_heap_used_bytes;
extern volatile uint32_t g_notification_extra_bytes;
extern volatile uint32_t g_ram_debug_mode;
extern volatile uint32_t g_ram_debug_ready;
extern volatile uint32_t g_tasks_debug_ready;
extern volatile uint32_t g_test_producer_count;
extern volatile uint32_t g_test_force_timeout;
extern volatile uint32_t g_sync_count;
extern volatile uint32_t g_timeout_count;
extern volatile uint32_t g_last_sync_period_ticks;
extern volatile uint32_t g_last_wakeup_cycles;
extern volatile uint32_t g_min_wakeup_cycles;
extern volatile uint32_t g_max_wakeup_cycles;
extern volatile uint32_t g_average_wakeup_cycles;

#endif /* SYNC_TASKS_H */
