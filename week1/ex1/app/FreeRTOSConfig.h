#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
#include "stm32f1xx.h"

extern uint32_t SystemCoreClock;

#define INCLUDE_uxTaskGetStackHighWaterMark 1
/* Scheduler */
#ifndef APP_USE_PREEMPTION
#define APP_USE_PREEMPTION                   1
#endif
#define configUSE_PREEMPTION                 APP_USE_PREEMPTION
#define configUSE_TIME_SLICING               1

/* CPU and kernel tick */
#define configCPU_CLOCK_HZ                   (SystemCoreClock)
#ifndef APP_TICK_RATE_HZ
#define APP_TICK_RATE_HZ 1000
#endif

#define configTICK_RATE_HZ APP_TICK_RATE_HZ

/* Task configuration */
#define configMAX_PRIORITIES                 5
#define configMINIMAL_STACK_SIZE            128
#define configMAX_TASK_NAME_LEN              16
#define configUSE_16_BIT_TICKS               0

/* Memory allocation */
#define configSUPPORT_DYNAMIC_ALLOCATION    1
#define configTOTAL_HEAP_SIZE               (8 * 1024)

/* Optional features */
#define configUSE_IDLE_HOOK                  0
#define configUSE_TICK_HOOK                  0
#define configUSE_TIMERS                     0
#define configUSE_MUTEXES                    0
#define configUSE_TRACE_FACILITY             0
#define configCHECK_FOR_STACK_OVERFLOW      0
#define configUSE_MALLOC_FAILED_HOOK        0

/* API inclusion */
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_vTaskDelayUntil              1
#define INCLUDE_vTaskSuspend                 1

/* Cortex-M3 interrupt configuration */
#define configPRIO_BITS                      __NVIC_PRIO_BITS

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Handler mapping */

#define vPortSVCHandler       SVC_Handler
#define xPortPendSVHandler    PendSV_Handler
#define xPortSysTickHandler   SysTick_Handler



/* Debug assertion */
#define configASSERT(x) \
    do { \
        if ((x) == 0) { \
            __disable_irq(); \
            for (;;) {} \
        } \
    } while (0)

#endif
