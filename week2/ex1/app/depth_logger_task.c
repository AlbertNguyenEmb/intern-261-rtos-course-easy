#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "pipeline.h"

#define DEPTH_LOG_PERIOD_MS    2U
#define DEPTH_LOG_CAPACITY     500U

volatile uint16_t g_depth_time_ms[DEPTH_LOG_CAPACITY];
volatile uint8_t g_raw_depth[DEPTH_LOG_CAPACITY];
volatile uint8_t g_filtered_depth[DEPTH_LOG_CAPACITY];

volatile uint32_t g_depth_sample_count;

void depth_logger_task(void *argument)
{
    TickType_t start_tick;
    TickType_t last_wake;

    (void)argument;

    start_tick = xTaskGetTickCount();
    last_wake = start_tick;

    for (;;) {
        uint32_t index = g_depth_sample_count;

        if (index < DEPTH_LOG_CAPACITY) {
            TickType_t now = xTaskGetTickCount();

            g_depth_time_ms[index] =
                (uint16_t)((now - start_tick) * portTICK_PERIOD_MS);

            g_raw_depth[index] =
                (uint8_t)uxQueueMessagesWaiting(queue_raw);

            g_filtered_depth[index] =
                (uint8_t)uxQueueMessagesWaiting(queue_filtered);

            /* Tăng count cuối cùng để đánh dấu mẫu đã ghi hoàn chỉnh. */
            g_depth_sample_count = index + 1U;
        } else {
            /*
             * Đã thu đủ 500 mẫu thì dừng logger để không tiếp tục
             * ảnh hưởng đến pipeline.
             */
            vTaskSuspend(NULL);
        }

        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(DEPTH_LOG_PERIOD_MS)
        );
    }
}