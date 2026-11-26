/* Week 4 / Ex2 -- data logger mini project, main implementation.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "mpu6050.h"
#include "sampler_isr.h"
#include "logger_task.h"
#include "datalogger_types.h"

static void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

extern QueueHandle_t log_queue;
void process_task(void *arg);
void board_uart_init(void);

void app_main(void)
{
    DWT_Init();
    mpu6050_init();
    board_uart_init();
    sampler_isr_init();

    /* TODO: xTaskCreate(logger_task, ...) at the priority you justified in
     *       week4/ex1/REPORT.md section 1. Do not pick priorities
     *       differently here without updating the design doc. */
    log_queue = xQueueCreate(32, sizeof(sensor_sample_t));
    configASSERT(log_queue != NULL);
    BaseType_t result = xTaskCreate(logger_task, "Logger", 256, NULL, 1, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(sampler_task, "Sampler", 256, NULL, 2, NULL);
    configASSERT(result == pdPASS);
    result = xTaskCreate(process_task, "Process", 256, NULL, 3, NULL);
    configASSERT(result == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
