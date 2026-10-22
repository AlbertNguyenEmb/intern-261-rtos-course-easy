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

volatile size_t g_heap_before_guard;
volatile size_t g_heap_after_guard;
volatile size_t g_heap_after_writers;
volatile size_t g_guard_heap_used;

void writer_task(void *argument);

void app_main(void)
{
    void *heap_probe;
    /* TODO: xTaskCreate WRITER_COUNT instances of writer_task, each given its
     *       own index (0..WRITER_COUNT-1) as the task parameter.
     *       Do not create WRITER_COUNT separately-named functions. */
    // Config count register
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    BaseType_t status;

    /* heap_4 initialises its accounting on the first allocation.  Perform a
     * temporary allocation so the before/after guard readings are valid. */
    heap_probe = pvPortMalloc(1U);
    configASSERT(heap_probe != NULL);
    vPortFree(heap_probe);

    g_heap_before_guard = xPortGetFreeHeapSize();

    uart_guard_init();

    g_heap_after_guard = xPortGetFreeHeapSize();
    g_guard_heap_used = g_heap_before_guard - g_heap_after_guard;
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
    g_heap_after_writers = xPortGetFreeHeapSize();
    vTaskStartScheduler();

    for (;;) {
    }
}
