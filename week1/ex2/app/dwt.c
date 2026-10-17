/* DWT cycle counter — the only honest way to burn a known number of
 * microseconds of CPU inside a task without blocking.
 */
#include "stm32f1xx.h"
#include "tasks.h"

void dwt_init(void)
{
    /* Enable the trace subsystem, clear the counter, then start CYCCNT. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable the TRCENA bit in the DEMCR register to activate DWT and ITM.
    DWT->CYCCNT = 0U; // Set the initial value count
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // enable DWT starting count
}

void dwt_busy_wait_us(uint32_t microseconds)
{
    /* Unsigned subtraction keeps the elapsed-time test valid across wrap. */
    const uint32_t start = DWT->CYCCNT; // cycles
    const uint32_t cycles = microseconds * (SystemCoreClock / 1000000U);// count the cycles that will be delayed

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
}
