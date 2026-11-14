#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include "FreeRTOS.h"
#include "event_groups.h"

extern EventGroupHandle_t supervisor_events;

extern volatile uint32_t g_wdg_feed_count;
extern volatile uint32_t g_wdg_miss_count;
extern volatile uint32_t g_boot_was_iwdg;
extern volatile TickType_t g_wdg_last_feed_tick;

/* Bit assignments -- one per monitored task, up to SUPERVISOR_BITS. */
#define SUPERVISOR_BIT_ALPHA   (1u << 0)
#define SUPERVISOR_BIT_BETA    (1u << 1)
#define SUPERVISOR_BIT_STATS  (1u << 2)
void supervisor_init(void);

/* Call from task_alpha/task_beta's loop once per iteration that makes real
 * progress (i.e. AFTER releasing both mutexes, not before taking them --
 * think about why that placement matters for what this is supposed to
 * detect). */
#define supervisor_checkin(bit) xEventGroupSetBits(supervisor_events, (bit))

void supervisor_task(void *argument);

#endif /* SUPERVISOR_H */
