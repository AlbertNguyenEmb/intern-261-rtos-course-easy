/* Week 2 / Ex3 — priority inversion.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "tasks_lmh.h"

#define LOW_PIN GPIO_PIN_0
#define MID_PIN GPIO_PIN_1
#define HIGH_PIN GPIO_PIN_2

#define LOW_PRIORITY      1U
#define MID_PRIORITY      2U
#define HIGH_PRIORITY     3U
#define TASK_STACK_WORDS  256U

static void gpio_init(void) {
    // 1. Bật xung nhịp Port A
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. Khai báo biến cấu hình
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 3. Thiết lập thông số
    GPIO_InitStruct.Pin = LOW_PIN | MID_PIN | HIGH_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      
    GPIO_InitStruct.Pull = GPIO_NOPULL;              
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;     

    HAL_GPIO_WritePin(
        GPIOA,
        LOW_PIN | MID_PIN | HIGH_PIN,
        GPIO_PIN_RESET
    );

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void app_main(void)
{
    /* TODO: xTaskCreate task_low, task_mid, task_high with three DISTINCT
     *       priorities, low < mid < high. Justify the exact numbers in
     *       REPORT.md — they must leave room for configMAX_PRIORITIES and not
     *       collide with any other task in the system. */
    gpio_init();

    // Cau hinh dwt
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    //Cau hinh mode tranh resource
    resource_init();

    BaseType_t status;

    status = xTaskCreate(task_low,"low", TASK_STACK_WORDS, NULL, LOW_PRIORITY, NULL);
    configASSERT(status == pdPASS);
    
    status = xTaskCreate(task_mid,"mid", TASK_STACK_WORDS, NULL, MID_PRIORITY, NULL);
    configASSERT(status == pdPASS);

    status = xTaskCreate(task_high,"high", TASK_STACK_WORDS, NULL, HIGH_PRIORITY, NULL);
    configASSERT(status == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
