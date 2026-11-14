/* Week 3 / Ex4 -- CPU load, deadlock, watchdog.
 * Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "params.h"
#include "runtime_stats.h"
#include "deadlock_demo.h"
#include "supervisor.h"

void app_main(void)
{
    runtime_stats_timer_init();
    deadlock_demo_init();
    supervisor_init();

    /* TODO: xTaskCreate task_alpha, task_beta, supervisor_task,
     *       runtime_stats_task. Run deadlock_demo exactly as given first
     *       (step 2 of README) before touching task_alpha/task_beta. */
    BaseType_t status;

    status = xTaskCreate(
        task_alpha, "alpha", 256, NULL, 2, NULL
    );
    configASSERT(status == pdPASS);

    status = xTaskCreate(
        task_beta, "beta", 256, NULL, 2, NULL
    );
    configASSERT(status == pdPASS);

    status = xTaskCreate(
        runtime_stats_task, "stats", 512, NULL, 1, NULL
    );
    configASSERT(status == pdPASS);

    status = xTaskCreate(
        supervisor_task, "supervisor",
        256, NULL, 3, NULL
    );
configASSERT(status == pdPASS);
    vTaskStartScheduler();

    for (;;) {
    }
}
