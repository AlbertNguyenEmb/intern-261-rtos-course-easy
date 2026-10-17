/* Week 1 / Ex2 — priorities, preemption, starvation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"
#define EX2_STACK_WORDS 128U
void app_main(void)
{
    BaseType_t result;

    dwt_init();

    result = xTaskCreate(task_a, "A", EX2_STACK_WORDS, NULL, TASK_A_PRIO, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_b, "B", EX2_STACK_WORDS, NULL, TASK_A_PRIO, NULL);
    configASSERT(result == pdPASS);

    result = xTaskCreate(task_c, "C", EX2_STACK_WORDS, NULL, TASK_A_PRIO, NULL);
    configASSERT(result == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}
