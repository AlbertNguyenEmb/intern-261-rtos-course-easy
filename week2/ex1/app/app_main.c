/* Week 2 / Ex1 — sensor pipeline: sensor_task -> queue_raw -> filter_task ->
 * queue_filtered -> uart_task.
 *
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

#define FILTER_TASK_PRIORITY  4U
#define SENSOR_TASK_PRIORITY  3U
#define LOGGER_TASK_PRIORITY  2U
#define UART_TASK_PRIORITY    1U

#define SENSOR_STACK_WORDS  256U
#define FILTER_STACK_WORDS  256U
#define UART_STACK_WORDS    384U
#define LOGGER_STACK_WORDS  256U

QueueHandle_t queue_raw;
QueueHandle_t queue_filtered;

void app_main(void)
{   
    /* TODO 1: xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t)) for queue_raw,
     *         and the equivalent for queue_filtered with filtered_sample_t.
     *         Check both handles are non-NULL before continuing. */
    BaseType_t status;

    queue_raw = xQueueCreate(QUEUE_LENGTH, sizeof(raw_sample_t));

    queue_filtered = xQueueCreate(QUEUE_LENGTH, sizeof(filtered_sample_t));

    configASSERT(queue_raw != NULL);
    configASSERT(queue_filtered != NULL);

    status = xTaskCreate(filter_task, "filter", FILTER_STACK_WORDS, NULL, FILTER_TASK_PRIORITY, NULL);
    configASSERT(status == pdPASS);

    /* TODO 2: xTaskCreate sensor_task, filter_task, uart_task.
     *         Pick priorities you can justify in REPORT.md: who must never be
     *         starved by whom? */
    status = xTaskCreate(sensor_task, "sensor", 256U, NULL, 2U, NULL);
    configASSERT(status == pdPASS);

    /* TODO 3: xTaskCreate depth_logger_task — must not perturb the pipeline it
     *         is measuring (think about its own priority and period). */
    status = xTaskCreate(uart_task, "uart", UART_STACK_WORDS, NULL, UART_TASK_PRIORITY, NULL);
    configASSERT(status == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
