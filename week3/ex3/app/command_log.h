#ifndef W3_EX3_COMMAND_LOG_H
#define W3_EX3_COMMAND_LOG_H

#include <stddef.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#ifndef W3EX3_SYNC_LOG
#define W3EX3_SYNC_LOG 0
#endif

typedef struct {
    uint32_t period_ms;
    UBaseType_t priority;
    eTaskState state;
} command_snapshot_t;

typedef enum {
    COMMAND_OK,
    COMMAND_BAD_FORMAT,
    COMMAND_BAD_NUMBER,
    COMMAND_OUT_OF_RANGE,
    COMMAND_TOO_LONG,
    COMMAND_STATS
} command_result_t;

/* Call once after uart_rx_init(), before starting the scheduler. */
void command_log_init(void);
/* Called by parser after RX starts; emits READY in synchronous test mode. */
void command_log_on_parser_start(void);

/* Parser task only. Mode 0: enqueue without waiting. Mode 1: format and
 * transmit here, deliberately delaying further parsing during the test. */
void command_log_submit(const char *cmd, size_t length,
                        const command_snapshot_t *before,
                        const command_snapshot_t *after,
                        command_result_t result);

#endif
