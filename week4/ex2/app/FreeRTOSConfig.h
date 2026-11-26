#include "../../../week1/ex1/app/FreeRTOSConfig.h"
#include "params.h"
#undef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE ((size_t)MAX_HEAP_BYTES)
#undef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY 1
#define configGENERATE_RUN_TIME_STATS 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() do { CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 
                                                      DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; \
                                                        } while (0)
#define portGET_RUN_TIME_COUNTER_VALUE() (DWT->CYCCNT)
