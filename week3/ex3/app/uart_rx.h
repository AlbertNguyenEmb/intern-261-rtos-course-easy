#ifndef UART_RX_H
#define UART_RX_H

#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "stream_buffer.h"

extern StreamBufferHandle_t rx_stream;
extern volatile uint32_t g_rx_bytes;
extern volatile uint32_t g_rx_dropped_bytes;
extern volatile uint32_t g_rx_rearm_errors;
extern volatile uint32_t g_rx_uart_errors;
extern volatile uint32_t g_rx_overruns;

/* Creates rx_stream and configures UART. Call before the scheduler. */
void uart_rx_init(void);

/* Called from HAL_UART_RxCpltCallback(huart) for USART1. Pushes the one
 * received byte into rx_stream and re-arms HAL_UART_Receive_IT() for the
 * next byte. */
void uart_rx_isr_handler(void);
void uart_rx_start(void);
/* Only the logger task may transmit. Shares the initialized USART1 handle. */
BaseType_t uart_tx_write(const char *data, size_t length);
#endif /* UART_RX_H */
