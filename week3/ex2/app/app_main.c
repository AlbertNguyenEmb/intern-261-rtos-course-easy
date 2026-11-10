/* Week 3 / Ex2 -- software timer debounce.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "debounce.h"
#include "uart_log.h"



static void button_gpio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_7 | GPIO_PIN_8,
        GPIO_PIN_RESET
    );

    gpio.Pin = GPIO_PIN_7 | GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);

}

static void enable_button_task(void *argument)
{
    (void)argument;

    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
    HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    vTaskSuspend(NULL);

    for (;;) {
    }
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_11) {
        uint8_t pin_is_high =
            HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)
            == GPIO_PIN_SET;

        debounce_on_edge(pin_is_high);
    }
}

void app_main(void)
{
    button_gpio_init();
    uart_log_init();
    debounce_init();

    BaseType_t status = xTaskCreate(
        enable_button_task,
        "btn_init",
        128,
        NULL,
        1,
        NULL
    );
    configASSERT(status == pdPASS);

    vTaskStartScheduler();

    for (;;) {
    }
}
