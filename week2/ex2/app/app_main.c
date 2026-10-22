/* Week 2 / Ex2 — race condition & mutex.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

#include "main.h"
#include "params.h"
#include "uart_guard.h"

#define WRITER_STACK_WORDS 256U
#define WRITER_PRIORITY    1U

void writer_task(void *argument);

void app_main(void)
{
    uart_guard_init();

    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */
    BaseType_t status;

    uart_guard_init();

    for (uint32_t i = 0U; i < WRITER_COUNT; i++) {
        status = xTaskCreate(
            writer_task,
            "writer",
            WRITER_STACK_WORDS,
            (void *)(uintptr_t)i,
            WRITER_PRIORITY,
            NULL
        );

        configASSERT(status == pdPASS);
    }
    vTaskStartScheduler();

    for (;;) {
    }
}
