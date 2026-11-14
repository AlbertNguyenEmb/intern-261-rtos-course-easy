/* Week 3 / Ex4 -- supervisor task: waits for all monitored tasks to check in
 * via an event group every cycle; refreshes IWDG only if they all did.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "supervisor.h"

EventGroupHandle_t supervisor_events;

#define ALL_BITS ((1u << SUPERVISOR_BITS) - 1u)
#define SUPERVISOR_WAIT_MS 750U

static IWDG_HandleTypeDef s_iwdg;

volatile uint32_t g_wdg_feed_count;
volatile uint32_t g_wdg_miss_count;
volatile uint32_t g_boot_was_iwdg;
volatile TickType_t g_wdg_last_feed_tick;

void supervisor_init(void)
{
    /* TODO 1: supervisor_events = xEventGroupCreate(); check non-NULL.
     *
     * TODO 2: HAL IWDG init here (IWDG_TIMEOUT_MS -> prescaler + reload per
     *         RM0008 IWDG chapter; IWDG runs off LSI, ~40 kHz, compute the
     *         reload value and show the computation in REPORT.md). */
     // Check nguyen nhan reset truoc khi xoa co
    g_boot_was_iwdg = (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET);

    __HAL_RCC_CLEAR_RESET_FLAGS();

    supervisor_events = xEventGroupCreate();
    configASSERT(supervisor_events != NULL);

    s_iwdg.Instance = IWDG;
    s_iwdg.Init.Prescaler = IWDG_PRESCALER_64;

    uint32_t counts =
        ((uint32_t)IWDG_TIMEOUT_MS * LSI_VALUE)
        / (64U * 1000U);

    configASSERT(counts >= 1U && counts <= 4096U);
    s_iwdg.Init.Reload = counts - 1U;

    HAL_StatusTypeDef status = HAL_IWDG_Init(&s_iwdg);
    configASSERT(status == HAL_OK);
}

void supervisor_task(void *argument)
{
    (void)argument;

    for (;;) {
        /* TODO 3: xEventGroupWaitBits(supervisor_events, ALL_BITS, pdTRUE,
         *         pdTRUE, <timeout>) -- clear-on-exit, wait-for-all. Pick a
         *         timeout tighter than IWDG_TIMEOUT_MS but loose enough that
         *         normal scheduling jitter does not cause a false trip;
         *         justify the number in REPORT.md.
         *
         * TODO 4: if the wait actually collected ALL_BITS (check the return
         *         value against ALL_BITS, do not assume success just because
         *         the call returned), HAL_IWDG_Refresh(&hiwdg). Otherwise do
         *         NOTHING -- that is what lets IWDG_TIMEOUT_MS expire and
         *         reset the board when a task stops checking in. */
        EventBits_t bits = xEventGroupWaitBits(supervisor_events, ALL_BITS, pdTRUE, pdTRUE, pdMS_TO_TICKS(SUPERVISOR_WAIT_MS));

        if ((bits & ALL_BITS) == ALL_BITS) {
            HAL_StatusTypeDef status =
                HAL_IWDG_Refresh(&s_iwdg);

            configASSERT(status == HAL_OK);

            g_wdg_last_feed_tick = xTaskGetTickCount();
            g_wdg_feed_count++;
        } else {
            g_wdg_miss_count++;
        }
    }
}
