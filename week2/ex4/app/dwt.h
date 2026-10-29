#ifndef DWT_H
#define DWT_H

#include <stdint.h>

void dwt_init(void);
uint32_t dwt_get_cycles(void);
uint32_t dwt_elapsed_cycles(uint32_t start_cycle);
uint32_t dwt_cycles_to_us(uint32_t cycles);

#endif /* DWT_H */
