/* Week 1 / Ex4 - print live RAM measurements through USART1 TX (PA9). */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "main.h"
#include "ram_report.h"

static void uart_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    USART1->BRR = HAL_RCC_GetPCLK2Freq() / 115200U;
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;
}

static void uart_putc(char character)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) { }
    USART1->DR = (uint32_t)(uint8_t)character;
}

static void uart_puts(const char *text)
{
    while (*text != '\0') {
        uart_putc(*text++);
    }
}

static void uart_put_u32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        uart_putc(digits[--count]);
    }
}

static void print_hwm(const char *name, TaskHandle_t task)
{
    UBaseType_t words = uxTaskGetStackHighWaterMark(task);

    uart_puts("HWM ");
    uart_puts(name);
    uart_puts(": ");
    uart_put_u32((uint32_t)words);
    uart_puts(" words, ");
    uart_put_u32((uint32_t)words * sizeof(StackType_t));
    uart_puts(" bytes\r\n");
}

void ram_report_task(void *argument)
{
    (void)argument;
    uart_init();

    for (;;) {
        uart_puts("\r\n=== RAM REPORT ===\r\n");
        uart_puts("Free heap: ");
        uart_put_u32((uint32_t)xPortGetFreeHeapSize());
        uart_puts(" bytes\r\n");

        uart_puts("Minimum ever free heap: ");
        uart_put_u32((uint32_t)xPortGetMinimumEverFreeHeapSize());
        uart_puts(" bytes\r\n");

        print_hwm("dyn_task", g_dyn_task_handle);
        print_hwm("stat_task", g_stat_task_handle);
        print_hwm("ram_report", g_ram_report_task_handle);
        print_hwm("idle", xTaskGetIdleTaskHandle());
        print_hwm("timer", xTimerGetTimerDaemonTaskHandle());
#if ADD_EXTRA_STATIC_TASK == 1
        print_hwm("extra_task", g_extra_task_handle);
#endif

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
