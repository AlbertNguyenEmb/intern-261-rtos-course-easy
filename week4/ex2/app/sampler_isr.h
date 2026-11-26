#ifndef SAMPLER_ISR_H
#define SAMPLER_ISR_H

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t sample_queue;

/* Configure TIM2 only; sampler_task starts it after its handle is valid. */
void sampler_isr_init(void);
void sampler_isr_start(void);
void sampler_task(void *arg);

/* Called from the timer ISR. Reads one sample and pushes it to
 * sample_queue. Runs in interrupt context -- every FreeRTOS call made here
 * MUST be the ...FromISR variant. */
void sampler_timer_callback(void);

#endif /* SAMPLER_ISR_H */
