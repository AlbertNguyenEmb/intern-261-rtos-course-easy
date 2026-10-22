/* Week 2 / Ex1 — moving-average filter, window = FILTER_WINDOW samples.
 * queue_raw -> queue_filtered.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

void filter_task(void *argument)
{   
    volatile uint32_t g_filter_processed_count;
    volatile uint32_t g_filtered_drop_count;

    int16_t ax_window[FILTER_WINDOW] = {0};
    int16_t ay_window[FILTER_WINDOW] = {0};
    int16_t az_window[FILTER_WINDOW] = {0};

    int32_t sum_ax = 0;
    int32_t sum_ay = 0;
    int32_t sum_az = 0;

    uint32_t index = 0;
    uint32_t count = 0;

    (void)argument;

    /* TODO 1: ring buffers for ax/ay/az, size FILTER_WINDOW, plus running sums.
     *         Do NOT recompute the full sum every sample — that is the whole
     *         point of a moving-average window. */

    for (;;) {
        raw_sample_t in;
        filtered_sample_t out;
        /* TODO 2: xQueueReceive(queue_raw, &in, portMAX_DELAY). */
        if (xQueueReceive(queue_raw, &in, portMAX_DELAY) != pdPASS) {
            continue;
        }
        /* TODO 3: push into the ring buffer, update running sums, compute the
         *         averaged ax/ay/az. */
        // Tru mau cu
        sum_ax -= ax_window[index];
        sum_ay -= ay_window[index];
        sum_az -= az_window[index];
        // Ghi mau moi tai index
        ax_window[index] = in.ax;
        ay_window[index] = in.ay;
        az_window[index] = in.az;
        // Cong mau moi vao tong de chia
        sum_ax += in.ax;
        sum_ay += in.ay;
        sum_az += in.az;

        if (count < FILTER_WINDOW) {
            count++;
        }
        index++;
        if (index >= FILTER_WINDOW) {
            index = 0;
        }
        out.seq = in.seq;
        
        /* TODO 4: fill out.ax/ay/az from the running average. */
        out.ax = (int16_t)(sum_ax / (int32_t)count);
        out.ay = (int16_t)(sum_ay / (int32_t)count);
        out.az = (int16_t)(sum_az / (int32_t)count);
        /* TODO 5: xQueueSend(queue_filtered, &out, <timeout>) — same question
         *         as sensor_task: what should happen here if uart_task is
         *         slow and queue_filtered is full? */
        if (xQueueSend(queue_filtered, &out, 0) != pdPASS) {
            g_filtered_drop_count++;
        }

        g_filter_processed_count++;
        (void)out;
    }
}
