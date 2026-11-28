/* Week 4 / Ex2 -- batches samples from sample_queue and writes them to
 * UART in blocks of LOG_BLOCK_SAMPLES.
 * Native FreeRTOS API only.
 */
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "datalogger_types.h"
#include "sampler_isr.h"
#include "logger_task.h"

extern UART_HandleTypeDef huart1;
extern QueueHandle_t log_queue;
volatile uint32_t uart_errors, uart_blocks;
volatile UBaseType_t logger_stack_free;

static sensor_sample_t block[LOG_BLOCK_SAMPLES];
volatile UBaseType_t queue_high_water_mark = 0;

void logger_task(void *argument)
{
    (void)argument;
    uint32_t filled = 0;

    for (;;) {
        /* TODO 1: xQueueReceive(log_queue, &block[filled], portMAX_DELAY).
         *
         * TODO 2: once filled == LOG_BLOCK_SAMPLES, HAL_UART_Transmit (or
         *         _DMA) the whole block out over USART1 at UART_BAUD, then
         *         reset filled = 0. Pick a wire format (binary struct dump
         *         or text) and document it in REPORT.md so your own
         *         decoding script in ex3 can parse it.
         *
         * TODO 3: if you need to know whether the queue is backing up
         *         (ex1 REPORT.md section 5), check uxQueueMessagesWaiting()
         *         here and report the high-water mark in week4/ex3. */

        if (xQueueReceive(log_queue,
                  &block[filled],
                  portMAX_DELAY) == pdTRUE) {
            static char tx_buf[LOG_BLOCK_SAMPLES * 64]; 
            filled++;

            if (filled == LOG_BLOCK_SAMPLES) {
            int offset = 0;
            for (uint32_t i = 0; i < LOG_BLOCK_SAMPLES; i++) {
                int n = snprintf(
                    tx_buf + offset,
                    sizeof(tx_buf) - (size_t)offset,
                    "%lu,%d,%d,%d,%d,%d,%d,%d\r\n",
                    (unsigned long)block[i].timestamp,
                    block[i].aceel_x, block[i].aceel_y, block[i].aceel_z,
                    block[i].temp,
                    block[i].gyro_x, block[i].gyro_y, block[i].gyro_z
                );
                if (n <= 0 || (size_t)(offset + n) >= sizeof(tx_buf)) {
                    break;
                }
                offset += n;
            }

            if (HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, (uint16_t)offset, 200) != HAL_OK) {
                uart_errors++;
            } else {
                uart_blocks++;
            }
            filled = 0;
        }
        }
        logger_stack_free = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t waiting = uxQueueMessagesWaiting(log_queue);

        if (waiting > queue_high_water_mark)
        {
            queue_high_water_mark = waiting;
        }
    }
}
