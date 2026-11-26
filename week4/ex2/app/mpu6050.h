#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>
#include "datalogger_types.h"

/* I2C1 init + MPU6050 wake-up (PWR_MGMT_1 = 0). */
void mpu6050_init(void);

/* Blocking read in sampler task only; false means invalid sample. */
bool mpu6050_read_sample(sensor_sample_t *out);

#endif /* MPU6050_H */
