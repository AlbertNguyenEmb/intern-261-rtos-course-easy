/* Week 1 / Ex4 — FreeRTOS hooks.
 *
 * Both hooks must be observable from outside the chip. A breakpoint is not a
 * deliverable: you need a UART line or a distinct LED pattern you can photograph.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"

#define ERROR_LED_PIN GPIO_PIN_5
static void error_blink(uint32_t pulse_count) {
    volatile uint32_t delay;

    __disable_irq(); // Tat cac phan ngat khac de uu tien bao loi cao nhat

    for (;;) {
        for (uint32_t pulse = 0; pulse < pulse_count; pulse++) {
            GPIOA->BSRR = ERROR_LED_PIN;// Muc cao
            for (delay = 0; delay < 300000U; delay++) {
            }

            GPIOA->BRR = ERROR_LED_PIN;// Muc thap
            for (delay = 0; delay < 300000U; delay++) {
            }
        }
        for (delay = 0; delay < 1200000U; delay++) {
        }
    }
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    error_blink(2U);
}

void vApplicationMallocFailedHook(void)
{
    error_blink(3U);
}

/* configSUPPORT_STATIC_ALLOCATION = 1 makes these two callbacks mandatory.
 * Provide the memory for the idle task and the timer task here. */
void vApplicationGetIdleTaskMemory(
    StaticTask_t **tcb_buffer,
    StackType_t **stack_buffer,
    uint32_t *stack_size)
{
    static StaticTask_t idle_tcb;
    static StackType_t idle_stack[configMINIMAL_STACK_SIZE];

    *tcb_buffer = &idle_tcb;
    *stack_buffer = idle_stack;
    *stack_size = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(
    StaticTask_t **tcb_buffer,
    StackType_t **stack_buffer,
    uint32_t *stack_size)
{
    static StaticTask_t timer_tcb;
    static StackType_t timer_stack[configTIMER_TASK_STACK_DEPTH];

    *tcb_buffer = &timer_tcb;
    *stack_buffer = timer_stack;
    *stack_size = configTIMER_TASK_STACK_DEPTH;
}
