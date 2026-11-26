/* Week 4 / Ex2 -- batches samples from sample_queue and writes them to
 * UART in blocks of LOG_BLOCK_SAMPLES.
 * Native FreeRTOS API only.
 */
#include <string.h>

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
            filled++;

            if (filled == LOG_BLOCK_SAMPLES) {
                if (HAL_UART_Transmit(
                    &huart1,
                    (uint8_t *)block,
                    sizeof(block),
                    100
                ) != HAL_OK) uart_errors++;
                else uart_blocks++;

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
