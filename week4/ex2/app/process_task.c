#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "datalogger_types.h"
#include "sampler_isr.h"
QueueHandle_t log_queue;
volatile uint32_t log_drop_count;
volatile UBaseType_t process_stack_free;
void process_task(void *arg) {
    (void)arg;
    sensor_sample_t sample;
    for (;;) {
        if (xQueueReceive(sample_queue, &sample, portMAX_DELAY) == pdPASS) {
            /* No filter specified: forward raw samples unchanged. */
            if (xQueueSend(log_queue, &sample, 0) != pdPASS) log_drop_count++;
            process_stack_free = uxTaskGetStackHighWaterMark(NULL);
        }
    }
}
