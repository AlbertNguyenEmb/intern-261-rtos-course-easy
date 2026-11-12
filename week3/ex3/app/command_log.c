#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "params.h"
#include "uart_rx.h"
#include "command_log.h"
#include "parser_task.h"

#define COMMAND_LOG_QUEUE_LENGTH 16U

typedef struct {
    char command[MAX_CMD_LEN];
    TickType_t tick;
    command_snapshot_t before;
    command_snapshot_t after;
    command_result_t result;
} command_record_t;

#if W3EX3_SYNC_LOG
#define LOG_MODE_NAME "SYNC_PARSER"
#else
#define LOG_MODE_NAME "ASYNC_TASK"
static QueueHandle_t s_log_queue;
#endif

/* A dropped TX log is counted separately from a dropped RX byte. */
volatile uint32_t g_command_logs_dropped;
volatile uint32_t g_command_log_tx_errors;
volatile uint32_t g_command_logs_sent;

static const char *state_name(eTaskState state)
{
    switch (state) {
    case eRunning:   return "RUNNING";
    case eReady:     return "READY";
    case eBlocked:   return "BLOCKED";
    case eSuspended: return "SUSPENDED";
    case eDeleted:   return "DELETED";
    default:         return "INVALID";
    }
}

static const char *result_name(command_result_t result)
{
    switch (result) {
    case COMMAND_OK:           return "OK";
    case COMMAND_BAD_FORMAT:   return "ERR_FORMAT";
    case COMMAND_BAD_NUMBER:   return "ERR_NUMBER";
    case COMMAND_OUT_OF_RANGE: return "ERR_RANGE";
    case COMMAND_TOO_LONG:     return "ERR_TOO_LONG";
    default:                   return "ERR_UNKNOWN";
    }
}

static void send_line(const char *line, int length, size_t capacity)
{
    if (length <= 0 || (size_t)length >= capacity ||
        uart_tx_write(line, (size_t)length) != pdPASS) {
        g_command_log_tx_errors++;
    } else {
        g_command_logs_sent++;
    }
}

static void print_ready(void)
{
    char line[384];

    int length = snprintf(
        line, sizeof(line),
        "READY log_mode=%s baud=%lu tick_hz=%lu stream=%lu trigger=%lu "
        "initial_period_ms=200 initial_priority=1 end=CR\r\n",
        LOG_MODE_NAME,
        (unsigned long)RX_BAUD, (unsigned long)configTICK_RATE_HZ,
        (unsigned long)STREAM_BUFFER_BYTES, (unsigned long)TRIGGER_LEVEL
    );
    send_line(line, length, sizeof(line));
}

/* Shared formatter: called by cmd_log in mode 0, parser itself in mode 1. */
static void transmit_record(command_record_t record)
{
    char line[384];
    int length;

    if (record.result == COMMAND_STATS) {
        /* Query after the burst: these are live cumulative counters. */
        length = snprintf(
            line, sizeof(line),
            "STATS mode=%s ok=%lu bad=%lu too_long=%lu seq_count=%lu "
            "first=%lu last=%lu seq_gap=%lu seq_order=%lu "
            "rx_bytes=%lu rx_drop=%lu uart_err=%lu ore=%lu "
            "rearm_err=%lu log_drop=%lu tx_err=%lu\r\n",
            LOG_MODE_NAME, (unsigned long)g_commands_ok,
            (unsigned long)g_commands_bad,
            (unsigned long)g_commands_too_long,
            (unsigned long)g_sequence_count,
            (unsigned long)g_sequence_first,
            (unsigned long)g_sequence_last,
            (unsigned long)g_sequence_gaps,
            (unsigned long)g_sequence_order_errors,
            (unsigned long)g_rx_bytes,
            (unsigned long)g_rx_dropped_bytes,
            (unsigned long)g_rx_uart_errors,
            (unsigned long)g_rx_overruns,
            (unsigned long)g_rx_rearm_errors,
            (unsigned long)g_command_logs_dropped,
            (unsigned long)g_command_log_tx_errors
        );
        send_line(line, length, sizeof(line));
        return;
    }

    length = snprintf(
        line, sizeof(line),
        "%s cmd=%s%s tick=%lu period_ms=%lu->%lu "
        "delay_ticks=%lu->%lu priority=%lu->%lu state=%s->%s "
        "rx_drop=%lu log_drop=%lu\r\n",
        result_name(record.result), record.command,
        record.result == COMMAND_TOO_LONG ? "..." : "",
        (unsigned long)record.tick,
        (unsigned long)record.before.period_ms,
        (unsigned long)record.after.period_ms,
        (unsigned long)pdMS_TO_TICKS(record.before.period_ms),
        (unsigned long)pdMS_TO_TICKS(record.after.period_ms),
        (unsigned long)record.before.priority,
        (unsigned long)record.after.priority,
        state_name(record.before.state), state_name(record.after.state),
        (unsigned long)g_rx_dropped_bytes,
        (unsigned long)g_command_logs_dropped
    );
    send_line(line, length, sizeof(line));
}

#if !W3EX3_SYNC_LOG
static void command_log_task(void *argument)
{
    (void)argument;
    print_ready();
    for (;;) {
        command_record_t record;
        if (xQueueReceive(s_log_queue, &record, portMAX_DELAY) == pdPASS) {
            transmit_record(record);
        }
    }
}
#endif

void command_log_init(void)
{
#if !W3EX3_SYNC_LOG
    s_log_queue = xQueueCreate(COMMAND_LOG_QUEUE_LENGTH,
                               sizeof(command_record_t));
    configASSERT(s_log_queue != NULL);

    BaseType_t status = xTaskCreate(
        command_log_task, "cmd_log", 512, NULL, 1, NULL
    );
    configASSERT(status == pdPASS);
#endif
}

void command_log_on_parser_start(void)
{
#if W3EX3_SYNC_LOG
    print_ready();
#endif
}

void command_log_submit(const char *cmd, size_t length,
                        const command_snapshot_t *before,
                        const command_snapshot_t *after,
                        command_result_t result)
{
#if !W3EX3_SYNC_LOG
    configASSERT(s_log_queue != NULL);
#endif
    command_record_t record = {0};
    record.tick = xTaskGetTickCount();
    record.before = *before;
    record.after = *after;
    record.result = result;

    size_t copied = 0U;
    while (copied < length && copied < sizeof(record.command) - 1U) {
        unsigned char ch = (unsigned char)cmd[copied];
        record.command[copied] = (ch >= 32U && ch <= 126U) ? (char)ch : '?';
        copied++;
    }
    record.command[copied] = '\0';

#if W3EX3_SYNC_LOG
    /* Experiment: parser cannot read another RX byte until TX finishes. */
    transmit_record(record);
#else
    if (xQueueSend(s_log_queue, &record, 0) != pdPASS) {
        g_command_logs_dropped++;
    }
#endif
}
