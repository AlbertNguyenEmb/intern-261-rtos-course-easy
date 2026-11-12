/* Week 3 / Ex3 -- UART RX -> stream buffer -> parser.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include "command_log.h"

void app_main(void)
{
    //Config PA8
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    gpio.Pin = GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
    uart_rx_init();
    command_log_init();

    /* Extra stack in sync mode holds the formatter and UART call chain. */
#if W3EX3_SYNC_LOG
    const configSTACK_DEPTH_TYPE parser_stack = 768;
#else
    const configSTACK_DEPTH_TYPE parser_stack = 384;
#endif
    TaskHandle_t demo_handle = NULL;

    BaseType_t status = xTaskCreate(demo_task, "demo", 256, NULL, 1, &demo_handle);
    configASSERT(status == pdPASS);

    status = xTaskCreate(parser_task, "parser", parser_stack, demo_handle, 4, NULL);
    configASSERT(status == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
