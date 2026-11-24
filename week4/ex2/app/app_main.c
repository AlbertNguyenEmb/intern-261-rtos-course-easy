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

void app_main(void)
{
    mpu6050_init();
    sampler_isr_init();

    /* TODO: xTaskCreate(logger_task, ...) at the priority you justified in
     *       week4/ex1/REPORT.md section 1. Do not pick priorities
     *       differently here without updating the design doc. */
    xTaskCreate()
    vTaskStartScheduler();

    for (;;) {
    }
}
