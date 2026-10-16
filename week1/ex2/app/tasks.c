/* Week 1 / Ex2 — the three competing tasks.
 *
 * Each task: raise its pin, burn exactly TASK_x_WORK_US of CPU, lower its pin,
 * and loop again WITHOUT blocking. The missing block is the point of the
 * exercise, not a bug to fix in step 2.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks.h"

static void run_task(GPIO_TypeDef *port, uint32_t pin, uint32_t work_us)
{
    const uint32_t pin_mask = 1UL << pin;

    for (;;) {
        port->BSRR = pin_mask;
        dwt_busy_wait_us(work_us);
        port->BSRR = pin_mask << 16U;
    }
}

void task_a(void *argument)
{
    (void)argument;
    run_task(GPIOA, TASK_A_PIN, TASK_A_WORK_US);
}

void task_b(void *argument)
{
    (void)argument;
    run_task(GPIOA, TASK_B_PIN, TASK_B_WORK_US);
}

void task_c(void *argument)
{
    (void)argument;
    run_task(GPIOA, TASK_C_PIN, TASK_C_WORK_US);
}
