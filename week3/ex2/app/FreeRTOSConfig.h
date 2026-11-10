#ifndef W3_EX2_FREERTOS_CONFIG_H
#define W3_EX2_FREERTOS_CONFIG_H

#include "../../../week1/ex1/app/FreeRTOSConfig.h"

#undef configUSE_TIMERS
#define configUSE_TIMERS                 1

#define configTIMER_TASK_PRIORITY        3
#define configTIMER_QUEUE_LENGTH         16
#define configTIMER_TASK_STACK_DEPTH     256

#endif