/* Week 2 / Ex3 — priority inversion vs priority inheritance.
 *
 * task_low  (lowest prio)  holds the shared resource for LOW_HOLD_MS.
 * task_mid  (middle prio)  never touches the resource; pure CPU-bound work,
 *                          its only job is to be ready and preemptible so it
 *                          can steal CPU from task_low while task_low holds
 *                          the resource task_high is waiting for.
 * task_high (highest prio) needs the resource every HIGH_PERIOD_MS and is
 *                          the one whose block time we measure.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

#define TRACE_GPIO_PORT GPIOA
#define LOW_TRACE_PIN   GPIO_PIN_0
#define MID_TRACE_PIN   GPIO_PIN_1
#define HIGH_TRACE_PIN  GPIO_PIN_2
#define WORK_SLICE_US 100U

static SemaphoreHandle_t s_resource;

/* Sampled only to prove priority inheritance: 1 with a binary semaphore,
 * temporarily 3 when Low owns a mutex needed by High. */
volatile UBaseType_t g_low_effective_priority;

void resource_init(void)
{
#if RESOURCE_MODE == 0
    /* TODO 1: xSemaphoreCreateBinary(), then give it once so it starts
     *         "available" (binary semaphores start empty). */
    s_resource = xSemaphoreCreateBinary();
    configASSERT(s_resource != NULL);
    // Da ton tai san key ban dau
    configASSERT(xSemaphoreGive(s_resource) == pdTRUE);
#else
    /* TODO 2: xSemaphoreCreateMutex(). */
    s_resource = xSemaphoreCreateMutex();
    configASSERT(s_resource != NULL);
#endif
}

static void busy_wait_us(uint32_t duration_us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t target_cycles = (SystemCoreClock / 1000000U) * duration_us;

    while ((uint32_t)(DWT->CYCCNT - start) < target_cycles) {
    }
}

static void cpu_work_ms(uint32_t duration_ms)
{
    uint32_t slices = duration_ms * 1000U / WORK_SLICE_US;

    for (uint32_t i = 0U; i < slices; i++) {
        busy_wait_us(WORK_SLICE_US);
    }
}

void task_low(void *argument)
{
    (void)argument;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        /* TODO 3: take s_resource (portMAX_DELAY), toggle a GPIO marking
         *         "resource held", busy-wait LOW_HOLD_MS using the ex2 DWT
         *         helper or a HAL tick-based wait, lower the GPIO, give
         *         s_resource back. Then yield/delay briefly before looping —
         *         this task must not monopolize the CPU outside the hold. */
        if (xSemaphoreTake(s_resource, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_WritePin(TRACE_GPIO_PORT, LOW_TRACE_PIN, GPIO_PIN_SET);
            // Doi va check priority de sau nay debug
            for (uint32_t i = 0U; i < (LOW_HOLD_MS * 10U); i++) {
                g_low_effective_priority =
                uxTaskPriorityGet(NULL);

                busy_wait_us(WORK_SLICE_US);
            }
            HAL_GPIO_WritePin(TRACE_GPIO_PORT, LOW_TRACE_PIN, GPIO_PIN_RESET);
            xSemaphoreGive(s_resource);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(HIGH_PERIOD_MS));
    }
}

void task_mid(void *argument)
{
    (void)argument;
    TickType_t last_wake = xTaskGetTickCount();
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(3U));
    for (;;) {
        /* TODO 4: toggle a GPIO, busy-wait MID_WORK_MS, lower the GPIO, then
         *         vTaskDelay a short period so it keeps becoming Ready
         *         periodically rather than running once and blocking
         *         forever. This task never touches s_resource. */
        HAL_GPIO_WritePin(TRACE_GPIO_PORT, MID_TRACE_PIN, GPIO_PIN_SET);
        cpu_work_ms(MID_WORK_MS);
        HAL_GPIO_WritePin(TRACE_GPIO_PORT, MID_TRACE_PIN, GPIO_PIN_RESET);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(HIGH_PERIOD_MS));
    }
}

void task_high(void *argument)
{
    (void)argument;
    TickType_t last_wake = xTaskGetTickCount();
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2U));
    for (;;) {
        /* TODO 5: toggle a GPIO high (marks "task_high wants the resource"),
         *         take s_resource (portMAX_DELAY), toggle a second GPIO
         *         transition (marks "task_high got it"), do a small amount
         *         of work, give s_resource back, lower the first GPIO.
         *         The gap between the two GPIO edges on the logic analyzer
         *         is your block time t1 - t0. */
        HAL_GPIO_WritePin(TRACE_GPIO_PORT, HIGH_TRACE_PIN, GPIO_PIN_SET);
        if (xSemaphoreTake(s_resource, portMAX_DELAY) == pdTRUE) {
            HAL_GPIO_WritePin(TRACE_GPIO_PORT, HIGH_TRACE_PIN, GPIO_PIN_RESET);
            busy_wait_us(100U);
            xSemaphoreGive(s_resource);
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(HIGH_PERIOD_MS));
    }
}
