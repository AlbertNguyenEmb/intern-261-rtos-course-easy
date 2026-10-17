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

#ifndef EX2_MODE
#define EX2_MODE 2
#endif

#ifndef EX2_C_WORK_MULTIPLIER
#define EX2_C_WORK_MULTIPLIER 1
#endif

static void gpio_work_task(GPIO_TypeDef *port, uint16_t pin, uint32_t work_us)
{
    for (;;) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        dwt_busy_wait_us(work_us);
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);

#if EX2_MODE == 5
        /* Blocking, unlike taskYIELD(), lets lower-priority Ready tasks run. */
        vTaskDelay(pdMS_TO_TICKS(1U));
#endif
    }
}

void task_a(void *argument)
{
    (void)argument;
    gpio_work_task(GPIOA, (uint16_t)(1U << TASK_A_PIN), TASK_A_WORK_US);
}

void task_b(void *argument)
{
    (void)argument;
    gpio_work_task(GPIOA, (uint16_t)(1U << TASK_B_PIN), TASK_B_WORK_US);
}

void task_c(void *argument)
{
    (void)argument;
    gpio_work_task(GPIOA, (uint16_t)(1U << TASK_C_PIN),
                   TASK_C_WORK_US * EX2_C_WORK_MULTIPLIER);
}
