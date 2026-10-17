/* Week 1 / Ex2 — priorities, preemption, starvation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

#define EX2_STACK_WORDS 128U

#ifndef EX2_MODE
#define EX2_MODE 2
#endif

#if EX2_MODE == 4
#define EX2_TASK_A_PRIORITY TASK_A_PRIO
#define EX2_TASK_B_PRIORITY TASK_A_PRIO
#define EX2_TASK_C_PRIORITY TASK_A_PRIO
#else
#define EX2_TASK_A_PRIORITY TASK_A_PRIO
#define EX2_TASK_B_PRIORITY TASK_B_PRIO
#define EX2_TASK_C_PRIORITY TASK_C_PRIO
#endif

void app_main(void)
{
    BaseType_t result;

    dwt_init();

    result = xTaskCreate(task_a, "A", EX2_STACK_WORDS, NULL,
                         EX2_TASK_A_PRIORITY, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_b, "B", EX2_STACK_WORDS, NULL,
                         EX2_TASK_B_PRIORITY, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_c, "C", EX2_STACK_WORDS, NULL,
                         EX2_TASK_C_PRIORITY, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}
