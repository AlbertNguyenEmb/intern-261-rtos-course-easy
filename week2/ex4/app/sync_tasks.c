/* Week 2 / Ex4 - event group and task notification measurements. */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "main.h"
#include "params.h"
#include "dwt.h"
#include "sync_tasks.h"

#define ALL_BITS_MASK ((((EventBits_t)1U << EVENT_BIT_COUNT) - (EventBits_t)1U))
#define REMAINING_BITS_MASK (ALL_BITS_MASK & ~(EventBits_t)1U)
#define UART_BAUD 115200U

_Static_assert(EVENT_BIT_COUNT > 1, "EVENT_BIT_COUNT must be greater than 1");
_Static_assert(EVENT_BIT_COUNT <= 24,
               "A 32-bit FreeRTOS event group has 24 usable bits");

static EventGroupHandle_t s_events;
static volatile uint32_t s_signal_cycle;
static uint64_t s_wakeup_cycle_sum;

void *sync_task_handle;

volatile uint32_t g_event_group_object_bytes;
volatile uint32_t g_event_group_heap_bytes;
volatile uint32_t g_sync_count;
volatile uint32_t g_timeout_count;
volatile uint32_t g_last_sync_period_ticks;
volatile uint32_t g_last_wakeup_cycles;
volatile uint32_t g_min_wakeup_cycles = UINT32_MAX;
volatile uint32_t g_max_wakeup_cycles;
volatile uint32_t g_average_wakeup_cycles;

static void uart_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    USART1->BRR = HAL_RCC_GetPCLK2Freq() / UART_BAUD;
    USART1->CR1 = USART_CR1_TE | USART_CR1_UE;
}

static void uart_putc(char value)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) {
    }
    USART1->DR = (uint32_t)(uint8_t)value;
}

static void uart_puts(const char *text)
{
    while (*text != '\0') {
        uart_putc(*text++);
    }
}

static void uart_put_u32(uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count > 0U) {
        uart_putc(digits[--count]);
    }
}

static void uart_put_hex(EventBits_t value)
{
    const char hex[] = "0123456789ABCDEF";

    uart_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        uart_putc(hex[(value >> shift) & 0x0FU]);
    }
}

static void update_wakeup_statistics(uint32_t cycles)
{
    g_last_wakeup_cycles = cycles;
    s_wakeup_cycle_sum += cycles;

    if (cycles < g_min_wakeup_cycles) {
        g_min_wakeup_cycles = cycles;
    }
    if (cycles > g_max_wakeup_cycles) {
        g_max_wakeup_cycles = cycles;
    }

    g_average_wakeup_cycles =
        (uint32_t)(s_wakeup_cycle_sum / g_sync_count);
}

void sync_init(void)
{
    size_t heap_before;

    uart_init();

    g_event_group_object_bytes = (uint32_t)sizeof(StaticEventGroup_t);
    heap_before = xPortGetFreeHeapSize();
    s_events = xEventGroupCreate();
    configASSERT(s_events != NULL);
    g_event_group_heap_bytes =
        (uint32_t)(heap_before - xPortGetFreeHeapSize());
}

void producer_task(void *argument)
{
    uint32_t idx = (uint32_t)(uintptr_t)argument;
    uint32_t random_state = 0x12345678U ^ ((idx + 1U) * 0x9E3779B9U);

    configASSERT(idx < EVENT_BIT_COUNT);

    for (;;) {
        uint32_t delay_ms;

        /* Xorshift32 gives each producer a small changing delay. */
        random_state ^= random_state << 13;
        random_state ^= random_state >> 17;
        random_state ^= random_state << 5;
        delay_ms = 1U + (random_state % NOTIFY_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

#if PRODUCER0_USE_NOTIFY
        if (idx == 0U) {
            s_signal_cycle = dwt_get_cycles();
            xTaskNotifyGive((TaskHandle_t)sync_task_handle);
        } else {
            xEventGroupSetBits(s_events, (EventBits_t)1U << idx);
        }
#else
        /* The producer that sets the last missing bit overwrites this value
         * immediately before it wakes sync_task. */
        s_signal_cycle = dwt_get_cycles();
        xEventGroupSetBits(s_events, (EventBits_t)1U << idx);
#endif
    }
}

void sync_task(void *argument)
{
    TickType_t previous_sync_tick = 0U;

    (void)argument;

    uart_puts("EX4 mode=");
#if PRODUCER0_USE_NOTIFY
    uart_puts("notification");
#else
    uart_puts("event-group");
#endif
    uart_puts(" object_bytes=");
    uart_put_u32(g_event_group_object_bytes);
    uart_puts(" heap_bytes=");
    uart_put_u32(g_event_group_heap_bytes);
    uart_puts("\r\n");

    for (;;) {
        EventBits_t bits;
        EventBits_t expected_bits;
        BaseType_t success;
        uint32_t wakeup_cycles;
        TickType_t now;

#if PRODUCER0_USE_NOTIFY
        uint32_t notified;

        notified = ulTaskNotifyTake(pdTRUE,
                                    pdMS_TO_TICKS(SYNC_TIMEOUT_MS));
        wakeup_cycles = dwt_elapsed_cycles(s_signal_cycle);

        expected_bits = REMAINING_BITS_MASK;
        bits = xEventGroupWaitBits(s_events, expected_bits,
                                   pdTRUE, pdTRUE,
                                   pdMS_TO_TICKS(SYNC_TIMEOUT_MS));
        success = ((notified > 0U) &&
                   ((bits & expected_bits) == expected_bits))
                      ? pdTRUE : pdFALSE;
#else
        expected_bits = ALL_BITS_MASK;
        bits = xEventGroupWaitBits(s_events, expected_bits,
                                   pdTRUE, pdTRUE,
                                   pdMS_TO_TICKS(SYNC_TIMEOUT_MS));
        wakeup_cycles = dwt_elapsed_cycles(s_signal_cycle);
        success = ((bits & expected_bits) == expected_bits)
                      ? pdTRUE : pdFALSE;
#endif

        now = xTaskGetTickCount();
        g_last_sync_period_ticks =
            (previous_sync_tick == 0U) ? 0U : (now - previous_sync_tick);
        previous_sync_tick = now;

        if (success == pdTRUE) {
            g_sync_count++;
            update_wakeup_statistics(wakeup_cycles);

            uart_puts("SYNC tick=");
            uart_put_u32((uint32_t)now);
            uart_puts(" period_ms=");
            uart_put_u32(g_last_sync_period_ticks * portTICK_PERIOD_MS);
            uart_puts(" wake_cycles=");
            uart_put_u32(g_last_wakeup_cycles);
            uart_puts(" wake_us=");
            uart_put_u32(dwt_cycles_to_us(g_last_wakeup_cycles));
            uart_puts("\r\n");
        } else {
            EventBits_t missing = expected_bits & ~bits;

            g_timeout_count++;
            uart_puts("TIMEOUT returned=");
            uart_put_hex(bits);
            uart_puts(" missing=");
            uart_put_hex(missing);
            uart_puts("\r\n");
        }
    }
}
