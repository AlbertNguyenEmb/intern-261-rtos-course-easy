/* Week 4 / Ex2 -- MPU6050 over I2C1.
 * Native FreeRTOS API only.
 */
#include "main.h"
#include "mpu6050.h"

static I2C_HandleTypeDef hi2c1;

#define MPU6050_ADDR        (0x68 << 1)
#define REG_PWR_MGMT_1       0x6B
#define REG_ACCEL_XOUT_H     0x3B

void mpu6050_init(void)
{
    /* TODO 1: HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, REG_PWR_MGMT_1, 1,
     *         &(uint8_t){0}, 1, 100) -- wake the sensor up out of sleep. */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
    uint8_t data = 0;

    if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, REG_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &data, 1, 100) != HAL_OK) Error_Handler();
    HAL_Delay(100);
}

bool mpu6050_read_sample(sensor_sample_t *out)
{
    /* TODO 2: HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, REG_ACCEL_XOUT_H, 1,
     *         buf, 14, 100) -- 14 bytes: accel x/y/z, temp, gyro x/y/z,
     *         big-endian. Fill out->timestamp with the DWT cycle counter
     *         (reuse runtime_stats_timer_value() style from week3/ex4) and
     *         the fields you added to sensor_sample_t in week4/ex1. */
    uint8_t buf[14];

    /* Capture timestamp */
    uint32_t timestamp = DWT->CYCCNT;

    /* Read 14 bytes from MPU6050 */
    if (HAL_I2C_Mem_Read(&hi2c1,
                        MPU6050_ADDR,
                        REG_ACCEL_XOUT_H,
                        I2C_MEMADD_SIZE_8BIT,
                        buf,
                        14,
                        2) != HAL_OK)
    {
        return false;
    }

    /* Timestamp */
    out->timestamp = timestamp;

    /* Accelerometer */
    out->aceel_x = (int16_t)((buf[0] << 8) | buf[1]);
    out->aceel_y = (int16_t)((buf[2] << 8) | buf[3]);
    out->aceel_z = (int16_t)((buf[4] << 8) | buf[5]);

    /* Temperature */
    out->temp = (int16_t)((buf[6] << 8) | buf[7]);

    /* Gyroscope */
    out->gyro_x = (int16_t)((buf[8]  << 8) | buf[9]);
    out->gyro_y = (int16_t)((buf[10] << 8) | buf[11]);
    out->gyro_z = (int16_t)((buf[12] << 8) | buf[13]);
    return true;
}
