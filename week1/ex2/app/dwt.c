/* DWT cycle counter — the only honest way to burn a known number of
 * microseconds of CPU inside a task without blocking.
 */
#include "stm32f1xx.h"
#include "tasks.h"

void dwt_init(void)
{
    /* TODO 1: enable the trace subsystem (CoreDebug DEMCR, TRCENA bit),
     *         zero DWT->CYCCNT, then enable CYCCNTENA.
     *         PM0056 does not document DWT; see the ARMv7-M ARM, C1.8. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable the TRCENA bit in the DEMCR register to activate DWT and ITM.
    DWT->CYCCNT = 0U; // Set the initial value count
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // enable DWT starting count
}

void dwt_busy_wait_us(uint32_t microseconds)
{
    /* TODO 2: spin until `microseconds` worth of CPU cycles have elapsed.
     *         SystemCoreClock is 72000000. Handle CYCCNT wrap-around correctly
     *         — unsigned subtraction, not a comparison of absolute values.
     *         Explain in REPORT.md why the naive comparison breaks every ~59 s. */
    (void)microseconds; //us
    const uint32_t start = DWT->CYCCNT; // cycles
    const uint32_t cycles = microseconds * (SystemCoreClock / 1000000U);// count the cycles that will be delayed

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
}
