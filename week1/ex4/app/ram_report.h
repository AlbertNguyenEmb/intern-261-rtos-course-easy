#ifndef RAM_REPORT_H
#define RAM_REPORT_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t g_dyn_task_handle;
extern TaskHandle_t g_stat_task_handle;
extern TaskHandle_t g_ram_report_task_handle;
#if ADD_EXTRA_STATIC_TASK == 1
extern TaskHandle_t g_extra_task_handle;
#endif

void ram_report_task(void *argument);

#endif
