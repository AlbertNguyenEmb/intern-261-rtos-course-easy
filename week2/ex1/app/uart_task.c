/* Week 2 / Ex1 — prints filtered samples to USART1 at UART_BAUD.
 * queue_filtered -> UART.
 *
 * This is also the task you deliberately starve in step 7 (temporarily set
 * UART_BAUD-equivalent hardware baud to 9600) to force queue_filtered to fill.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#include "main.h"
#include "params.h"
#include "pipeline.h"
#include <string.h>

static UART_HandleTypeDef huart1;

volatile uint32_t g_uart_received_count;// so filterd sample da lay khoi queue
volatile uint32_t g_uart_gap_count;// tong so seq bi thieu
volatile uint32_t g_uart_error_count;// so lan format hoac truyen uart that bai

static HAL_StatusTypeDef uart1_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = UART_BAUD;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    return HAL_UART_Init(&huart1);
}

void uart_task(void *argument)
{
    uint32_t previous_seq = 0U;
    uint8_t have_previous = 0U;
    HAL_StatusTypeDef status;

    (void)argument;

    status = uart1_init();

    if (status != HAL_OK) {
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    for (;;) {
        filtered_sample_t in;
        char line[48];
        int length;

        if (xQueueReceive(
                queue_filtered,
                &in,
                portMAX_DELAY
            ) != pdPASS) {
            continue;
        }

        if (have_previous != 0U) {
            uint32_t expected_seq = previous_seq + 1U;

            if (in.seq > expected_seq) {
                g_uart_gap_count += in.seq - expected_seq;
            }
        }

        previous_seq = in.seq;
        have_previous = 1U;
        g_uart_received_count++;

        length = snprintf(
            line,
            sizeof(line),
            "%lu,%d,%d,%d\r\n",
            (unsigned long)in.seq,
            (int)in.ax,
            (int)in.ay,
            (int)in.az
        );

        if ((length <= 0) ||
            ((size_t)length >= sizeof(line))) {
            g_uart_error_count++;
            continue;
        }

        status = HAL_UART_Transmit(
            &huart1,
            (uint8_t *)line,
            (uint16_t)length,
            100U
        );

        if (status != HAL_OK) {
            g_uart_error_count++;
        }
    }
}
