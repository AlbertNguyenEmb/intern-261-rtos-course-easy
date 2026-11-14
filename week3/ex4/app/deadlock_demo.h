#ifndef DEADLOCK_DEMO_H
#define DEADLOCK_DEMO_H

extern volatile unsigned long g_alpha_cycles;
extern volatile unsigned long g_beta_cycles;

void deadlock_demo_init(void);
void task_alpha(void *argument);
void task_beta(void *argument);

#endif /* DEADLOCK_DEMO_H */
