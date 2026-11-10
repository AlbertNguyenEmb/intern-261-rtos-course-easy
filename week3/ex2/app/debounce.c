/* Week 3 / Ex2 -- debounce via one-shot timer + long-press via auto-reload
 * timer. Native FreeRTOS API only.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "main.h"
#include "params.h"
#include "debounce.h"
#include "uart_log.h"

static TimerHandle_t s_debounce_timer;
static TimerHandle_t s_longpress_timer;
static uint8_t s_pressed;

static void debounce_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    TickType_t stamp = xTaskGetTickCount();
    uint8_t pressed = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET;

    if (pressed == s_pressed) {
        return;
    }

    s_pressed = pressed;

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_8,
        pressed ? GPIO_PIN_SET : GPIO_PIN_RESET
    );

    uart_log_event(pressed ? UART_LOG_PRESS : UART_LOG_RELEASE, stamp);
    vTaskDelay(50);
}

static void longpress_callback(TimerHandle_t xTimer)
{
    (void)xTimer;

    TickType_t stamp = xTaskGetTickCount();
    if (!s_pressed || HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) != GPIO_PIN_RESET) {
        return;
    }
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
    uart_log_event(UART_LOG_HELD, stamp);
}

void debounce_init(void)
{
    /* Timers remain dormant until an EXTI edge sends their first command. */
    s_debounce_timer = xTimerCreate("DB", pdMS_TO_TICKS(DEBOUNCE_MS), pdFALSE, NULL, debounce_callback);
    configASSERT(s_debounce_timer);

    s_longpress_timer = xTimerCreate("LP", pdMS_TO_TICKS(LONGPRESS_MS), pdTRUE, NULL, longpress_callback);
    configASSERT(s_longpress_timer);
}

void debounce_on_edge(uint8_t pin_is_high)
{
    BaseType_t task_woken = pdFALSE;
    BaseType_t status;

    /* Every new edge restarts the debounce countdown. */
    status = xTimerResetFromISR(s_debounce_timer, &task_woken);
    configASSERT(status == pdPASS);

    /* Pull-up wiring: low = pressed, high = released. */
    if (!pin_is_high) {
        status = xTimerStartFromISR(s_longpress_timer, &task_woken);
    } else {
        status = xTimerStopFromISR(s_longpress_timer, &task_woken);
    }
    configASSERT(status == pdPASS);
    portYIELD_FROM_ISR(task_woken);
}
