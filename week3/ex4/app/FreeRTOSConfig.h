#ifndef W3_EX4_FREERTOS_CONFIG_H
#define W3_EX4_FREERTOS_CONFIG_H

#include "../../../week1/ex1/app/FreeRTOSConfig.h"
#include "runtime_stats.h"

/* Hai task sử dụng mutex. */
#undef configUSE_MUTEXES
#define configUSE_MUTEXES 1

/* Thu thập thông tin task và run-time statistics. */
#undef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY 1

#define configGENERATE_RUN_TIME_STATS        1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

/* Các API hỗ trợ chẩn đoán. */
#define INCLUDE_eTaskGetState             1
#define INCLUDE_xSemaphoreGetMutexHolder  1

/* Nguồn thời gian dùng để thống kê CPU. */
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() \
    runtime_stats_timer_init()

#define portGET_RUN_TIME_COUNTER_VALUE() \
    runtime_stats_timer_value()

#endif