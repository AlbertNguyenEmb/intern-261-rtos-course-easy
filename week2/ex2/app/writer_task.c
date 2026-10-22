/* Week 2 / Ex2 — WRITER_COUNT identical writer tasks. Each prints an MSG_LEN
 * byte string with its own id + sequence number, every WRITER_PERIOD_MS.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"

volatile uint32_t g_latency_last_us[WRITER_COUNT];
volatile uint32_t g_latency_avg_us[WRITER_COUNT];
volatile uint32_t g_latency_max_us[WRITER_COUNT];
volatile uint32_t g_latency_count[WRITER_COUNT];
volatile uint64_t g_latency_sum_us[WRITER_COUNT];

/* TODO 1: one writer_task(void *argument) taking the writer index via the
 *         task parameter (argument), NOT WRITER_COUNT copy-pasted bodies.
 *         Format a MSG_LEN-byte string containing the writer id and a
 *         per-writer sequence number, then uart_guard_send() it, then
 *         vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS)). */
void writer_task(void *argument)
{
    uint32_t id = (uint32_t)(uintptr_t)argument;
    uint32_t seq = 0;
    (void)id;

    for (;;) {
        /* TODO 2 */
        char msg[MSG_LEN];
        int written;
        size_t used;
        // Ghi id va seq ra bo dem msg
        written = snprintf(msg, MSG_LEN, "[W%lu seq=%06lu]", (unsigned long)id, (unsigned long)seq);
        if (written < 0) {
            used = 0U;
        } else if ((size_t)written > (MSG_LEN - 2U)) {
            used = MSG_LEN - 2U;
        } else {
            used = (size_t)written;
        }

        while (used < (MSG_LEN - 2U)) {
            msg[used] = (char)('A' + id);
            used++;
        }
        msg[MSG_LEN - 2U] = '\r';
        msg[MSG_LEN - 1U] = '\n';
        
        uint32_t start_cycles; // cycle bat dau 
        uint32_t elapsed_cycles;
        uint32_t elapsed_us;
        // Ghi lai thoi diem bat dau
        start_cycles = DWT->CYCCNT;

        uart_guard_send(msg, MSG_LEN);
        // Tinh thoi gian troi qua
        elapsed_cycles = DWT->CYCCNT - start_cycles;
        // Quy doi sang thoi gian thuc
        elapsed_us = elapsed_cycles / (SystemCoreClock / 1000000U);

        g_latency_last_us[id] = elapsed_us; // Tgian gan nhat 
        g_latency_sum_us[id] += elapsed_us; // Tong thoi gian do
        g_latency_count[id]++; // so lan do
        g_latency_avg_us[id] = (uint32_t)(
            g_latency_sum_us[id] / g_latency_count[id]
        );

        if (elapsed_us > g_latency_max_us[id]) {
            g_latency_max_us[id] = elapsed_us;
        }

        
        seq++;
        (void)seq;
        vTaskDelay(pdMS_TO_TICKS(WRITER_PERIOD_MS));
    }
}
