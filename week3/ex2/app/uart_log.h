#ifndef W3_EX2_UART_LOG_H
#define W3_EX2_UART_LOG_H

#include "FreeRTOS.h"

typedef enum {
    UART_LOG_PRESS,
    UART_LOG_RELEASE,
    UART_LOG_HELD
} uart_log_event_t;

/* Before the scheduler: configure USART1 TX on PA9 and create the logger. */
void uart_log_init(void);

/* Task/timer-callback context only. Never waits for queue space.
 * Capture tick at the event, rather than when UART eventually transmits it. */
BaseType_t uart_log_event(uart_log_event_t event, TickType_t tick);

#endif
