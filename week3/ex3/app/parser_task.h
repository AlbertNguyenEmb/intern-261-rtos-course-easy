#ifndef PARSER_TASK_H
#define PARSER_TASK_H

#include <stdint.h>

extern volatile uint32_t g_commands_ok;
extern volatile uint32_t g_commands_bad;
extern volatile uint32_t g_commands_too_long;
extern volatile uint32_t g_sequence_count;
extern volatile uint32_t g_sequence_first;
extern volatile uint32_t g_sequence_last;
extern volatile uint32_t g_sequence_gaps;
extern volatile uint32_t g_sequence_order_errors;

void parser_task(void *argument);
void demo_task(void *argument);

#endif /* PARSER_TASK_H */
