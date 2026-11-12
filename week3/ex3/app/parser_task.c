#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "main.h"
#include "params.h"
#include "uart_rx.h"
#include "parser_task.h"
#include "command_log.h"

static TaskHandle_t s_demo_task_handle;
static volatile uint32_t s_demo_period_ms = 200;

volatile uint32_t g_commands_ok;
volatile uint32_t g_commands_bad;
volatile uint32_t g_commands_too_long;
volatile uint32_t g_sequence_count;
volatile uint32_t g_sequence_first;
volatile uint32_t g_sequence_last;
volatile uint32_t g_sequence_gaps;
volatile uint32_t g_sequence_order_errors;

static command_snapshot_t demo_snapshot(void)
{
    command_snapshot_t snapshot;
    snapshot.period_ms = s_demo_period_ms;
    snapshot.priority = uxTaskPriorityGet(s_demo_task_handle);
    snapshot.state = eTaskGetState(s_demo_task_handle);
    return snapshot;
}

static command_result_t parse_decimal(const char *text, size_t len,
                                      uint32_t *result)
{
    if (len == 0U) {
        return COMMAND_BAD_NUMBER;
    }
    uint32_t value = 0;
    for (size_t i = 0U; i < len; i++) {
        if (text[i] < '0' || text[i] > '9') {
            return COMMAND_BAD_NUMBER;
        }
        uint32_t digit = (uint32_t)(text[i] - '0');
        if (value > (UINT32_MAX - digit) / 10U) {
            return COMMAND_OUT_OF_RANGE;
        }
        value = value * 10 + digit;
    }
    *result = value;
    return COMMAND_OK;
}

static command_result_t execute_command(const char *cmd, size_t len)
{
    if (cmd == NULL || len < 2 || s_demo_task_handle == NULL) {
        return COMMAND_BAD_FORMAT;
    }

    /* Optional sequence suffix: P10#42, S2#43, T0#44. */
    size_t command_len = 1U;
    while (command_len < len && cmd[command_len] != '#') {
        command_len++;
    }
    uint32_t value;
    command_result_t parsed = parse_decimal(cmd + 1, command_len - 1U, &value);
    if (parsed != COMMAND_OK) {
        return parsed;
    }
    uint32_t sequence = 0U;
    BaseType_t has_sequence = command_len < len;
    if (has_sequence) {
        parsed = parse_decimal(cmd + command_len + 1U,
                               len - command_len - 1U, &sequence);
        if (parsed != COMMAND_OK) {
            return parsed;
        }
    }

    switch (cmd[0]) {
        case 'P': {
            uint64_t ticks = (uint64_t)value * configTICK_RATE_HZ / 1000;

            if (ticks == 0 || ticks >= (uint64_t)portMAX_DELAY) {
                return COMMAND_OUT_OF_RANGE;
            }
            s_demo_period_ms = value;
            break;
        }

        case 'S': {
            if (value >= configMAX_PRIORITIES) {
                return COMMAND_OUT_OF_RANGE;
            }

            vTaskPrioritySet(s_demo_task_handle, (UBaseType_t)value);
            break;
        }

        case 'T':
            if (command_len != 2U || value > 1U) {
                return COMMAND_OUT_OF_RANGE;
            }

            if (value == 0) {
                vTaskSuspend(s_demo_task_handle);
            } else if (value == 1) {
                vTaskResume(s_demo_task_handle);
            }
            break;

        default:
            return COMMAND_BAD_FORMAT;
    }
    if (has_sequence) {
        /* Each burst starts at zero after a board reset; no ID wraparound. */
        if (g_sequence_count == 0U) {
            g_sequence_first = sequence;
            g_sequence_gaps = sequence;
        } else if (sequence > g_sequence_last) {
            g_sequence_gaps += sequence - g_sequence_last - 1U;
        } else {
            g_sequence_order_errors++;
        }
        g_sequence_last = sequence;
        g_sequence_count++;
    }
    return COMMAND_OK;
}

static void apply_command(const char *cmd, size_t len)
{
    command_snapshot_t before = demo_snapshot();
    command_result_t result = execute_command(cmd, len);
    command_snapshot_t after = demo_snapshot();

    if (result == COMMAND_OK) {
        g_commands_ok++;
    } else {
        g_commands_bad++;
    }

    /* Mode 0 queues this record; mode 1 deliberately transmits in parser. */
    command_log_submit(cmd, len, &before, &after, result);
}

void parser_task(void *argument)
{
    s_demo_task_handle = (TaskHandle_t)argument;
    configASSERT(s_demo_task_handle != NULL);

    uart_rx_start();
    command_log_on_parser_start();
    char cmd[MAX_CMD_LEN];
    size_t idx = 0;

    BaseType_t discarding = pdFALSE;
    for (;;) {
        uint8_t byte;

        size_t received = xStreamBufferReceive(rx_stream, &byte, 1, portMAX_DELAY);

        if (received != 1) {
            continue;
        }
        if (byte == '\r') {
            cmd[idx] = '\0';
            if (discarding != pdFALSE) {
                command_snapshot_t snapshot = demo_snapshot();
                g_commands_bad++;
                command_log_submit(cmd, idx, &snapshot, &snapshot,
                                   COMMAND_TOO_LONG);
            } else if (idx == 1U && cmd[0] == '?') {
                command_snapshot_t snapshot = demo_snapshot();
                /* Query is excluded from the number of tested commands. */
                command_log_submit(cmd, idx, &snapshot, &snapshot,
                                   COMMAND_STATS);
            } else if (idx > 0U) {
                apply_command(cmd, idx);
            }
            idx = 0;
            discarding = pdFALSE;
        } else if (discarding != pdFALSE) {
            /* Keep the prefix for the error log; ignore the rest until CR. */
            continue;
        } else if (byte == '\n' && idx == 0U) {
            /* Ignore the LF following a terminal's CRLF line ending. */
            continue;
        } else if (idx < sizeof(cmd) - 1U) {
            cmd[idx++] = (char)byte;
        } else {
            g_commands_too_long++;
            discarding = pdTRUE;
        }
    }
}

void demo_task(void *argument)
{
    (void)argument;

    for (;;) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
        vTaskDelay(pdMS_TO_TICKS(s_demo_period_ms));
    }
}
