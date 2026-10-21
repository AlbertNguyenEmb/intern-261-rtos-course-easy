/* Week 1 / Ex4 — entry point. */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

#define RAM_REPORT_STACK_WORDS 320U
#define WORKER_PRIORITY         1U
#define REPORT_PRIORITY         2U
#define SAFE_DYN_TASK_STACK_WORDS 72U

#define ENABLE_STACK_HOG 0

/* Handles are kept so ram_report_task can measure each task. */
TaskHandle_t g_dyn_task_handle;
TaskHandle_t g_stat_task_handle;
TaskHandle_t g_ram_report_task_handle;

/* TCB and stack supplied by the application for stat_task. */
static StaticTask_t stat_task_tcb;
static StackType_t stat_task_stack[STATIC_TASK_STACK_WORDS];

#if ADD_EXTRA_STATIC_TASK == 1
TaskHandle_t g_extra_task_handle;
static StaticTask_t extra_task_tcb;
static StackType_t extra_task_stack[128U];
#endif

void ram_report_task(void *argument);

static void shared_work(void *argument)
{
    (void)argument;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void dynamic_work(void *argument)
{
    (void)argument;

#if ENABLE_STACK_HOG
    volatile uint8_t stack_hog[HOG_DEPTH_BYTES];
#endif

    for (;;) {
#if ENABLE_STACK_HOG
        for (uint32_t i = 0; i < HOG_DEPTH_BYTES; i++) {
            stack_hog[i] = (uint8_t)i;
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    BaseType_t status;
    status = xTaskCreate(dynamic_work, "dyn_task", SAFE_DYN_TASK_STACK_WORDS, NULL, WORKER_PRIORITY, &g_dyn_task_handle);
    if (status != pdPASS) {
        vApplicationMallocFailedHook();
    }
    /* xTaskCreateStatic returns a handle instead of pdPASS/pdFAIL. */
    g_stat_task_handle = xTaskCreateStatic(shared_work, "stat_task", STATIC_TASK_STACK_WORDS, NULL, WORKER_PRIORITY, stat_task_stack, &stat_task_tcb);
    configASSERT(g_stat_task_handle != NULL);

#if ADD_EXTRA_STATIC_TASK == 1
    g_extra_task_handle = xTaskCreateStatic(
        shared_work, "extra_task", 128U, NULL, WORKER_PRIORITY,
        extra_task_stack, &extra_task_tcb);
    configASSERT(g_extra_task_handle != NULL);
#endif

    status = xTaskCreate(ram_report_task, "ram_report", RAM_REPORT_STACK_WORDS, NULL, REPORT_PRIORITY, &g_ram_report_task_handle);
    if (status != pdPASS) {
        vApplicationMallocFailedHook();
    }
    vTaskStartScheduler();
    /* Scheduler returned => not enough heap for the idle/timer task. */
    for (;;) {
    }
}
