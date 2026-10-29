/* Cortex-M3 DWT cycle counter helpers. */
#include "main.h"
#include "dwt.h"

void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t dwt_get_cycles(void)
{
    return DWT->CYCCNT;
}

uint32_t dwt_elapsed_cycles(uint32_t start_cycle)
{
    /* Unsigned subtraction is also correct when CYCCNT wraps once. */
    return DWT->CYCCNT - start_cycle;
}

uint32_t dwt_cycles_to_us(uint32_t cycles)
{
    return cycles / (SystemCoreClock / 1000000U);
}
