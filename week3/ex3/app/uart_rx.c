/* Week 3 / Ex3 -- UART RX interrupt -> stream buffer.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"

static UART_HandleTypeDef huart1;

volatile uint32_t g_rx_bytes;
volatile uint32_t g_rx_dropped_bytes;
volatile uint32_t g_rx_rearm_errors;
volatile uint32_t g_rx_uart_errors;
volatile uint32_t g_rx_overruns;
StreamBufferHandle_t rx_stream;

static uint8_t s_rx_byte;

void uart_rx_init(void)
{
    /* RX is armed later by parser_task, after its target handle is valid. */
    rx_stream = xStreamBufferCreate(192, TRIGGER_LEVEL);
    configASSERT(rx_stream != NULL);
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* PA9: UART TX. */
    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA10: UART RX. */
    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = RX_BAUD;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef status = HAL_UART_Init(&huart1);
    configASSERT(status == HAL_OK);

    HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
}

void uart_rx_start(void)
{
    HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    HAL_StatusTypeDef status =
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);

    configASSERT(status == HAL_OK);
}

void uart_rx_isr_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    g_rx_bytes++;
    size_t sent = xStreamBufferSendFromISR(rx_stream, &s_rx_byte, 1, &xHigherPriorityTaskWoken);

    if (sent != 1) {
        g_rx_dropped_bytes++;
    } else if (s_rx_byte == '\r') {
        /* A complete short command must not wait for four pending bytes. */
        (void)xStreamBufferSendCompletedFromISR(
            rx_stream, &xHigherPriorityTaskWoken
        );
    }

    if (HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) != HAL_OK) {
        g_rx_rearm_errors++;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

BaseType_t uart_tx_write(const char *data, size_t length)
{
    if (data == NULL || length == 0U || length > UINT16_MAX) {
        return pdFAIL;
    }
    return HAL_UART_Transmit(&huart1, (uint8_t *)data,
                            (uint16_t)length, 100U) == HAL_OK ? pdPASS : pdFAIL;
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_rx_isr_handler();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) {
        return;
    }
    g_rx_uart_errors++;
    if ((huart->ErrorCode & HAL_UART_ERROR_ORE) != 0U) {
        g_rx_overruns++;
    }

    /* HAL aborts RX for an overrun. Restart only if it has ended reception;
     * nonblocking framing/noise errors may leave the current RX active. */
    if (huart->RxState == HAL_UART_STATE_READY &&
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1) != HAL_OK) {
        g_rx_rearm_errors++;
    }
}
