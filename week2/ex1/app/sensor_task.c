/* Week 2 / Ex1 — reads MPU6050 over I2C1 every SAMPLE_PERIOD_MS, pushes a
 * raw_sample_t into queue_raw.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "main.h"
#include "params.h"
#include "pipeline.h"

#define MPU6050_ADDRESS          (0x68U << 1)
#define MPU6050_REG_PWR_MGMT_1   0x6BU
#define MPU6050_REG_WHO_AM_I     0x75U
#define MPU6050_REG_ACCEL_XOUT   0x3BU

static I2C_HandleTypeDef hi2c1;

volatile uint8_t g_mpu_who_am_i;
volatile HAL_StatusTypeDef g_mpu_status;

volatile uint32_t g_sensor_read_ok_count; //So lan doc i2c thanh cong
volatile uint32_t g_i2c_error_count; // so trấnction doc gia toc bi loi
volatile uint32_t g_raw_drop_count; // So mau doc duoc nhung k gui dc

static HAL_StatusTypeDef mpu_i2c_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000U;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0U;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0U;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return HAL_I2C_Init(&hi2c1);
}

static HAL_StatusTypeDef mpu6050_probe(void)
{
    uint8_t who_am_i = 0U;
    uint8_t wake_value = 0U;
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(
        &hi2c1,
        MPU6050_ADDRESS,
        MPU6050_REG_WHO_AM_I,
        I2C_MEMADD_SIZE_8BIT,
        &who_am_i,
        1U,
        20U
    );

    if (status != HAL_OK) {
        return status;
    }

    g_mpu_who_am_i = who_am_i;

    if (who_am_i != 0x68U) {
        return HAL_ERROR;
    }

    status = HAL_I2C_Mem_Write(
        &hi2c1,
        MPU6050_ADDRESS,
        MPU6050_REG_PWR_MGMT_1,
        I2C_MEMADD_SIZE_8BIT,
        &wake_value,
        1U,
        20U
    );

    return status;
}

void sensor_task(void *argument)
{
    (void)argument;
    uint32_t seq = 0;

    /* TODO 1: HAL_I2C init / verify MPU6050 WHO_AM_I. Do this once here, not
     *         inside the loop. */
    g_mpu_status = mpu_i2c_init();

    if (g_mpu_status == HAL_OK) {
        g_mpu_status = mpu6050_probe();
    }
    // Neu khoi dong loi thi se dung o trang thai an toan
    if (g_mpu_status != HAL_OK) {
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        uint8_t data[6];
        raw_sample_t s;
        s.seq = seq++;
        HAL_StatusTypeDef status;
        /* TODO 2: HAL_I2C_Mem_Read the accel registers into s.ax/ay/az. */
        status = HAL_I2C_Mem_Read(
            &hi2c1,
            MPU6050_ADDRESS,
            MPU6050_REG_ACCEL_XOUT,
            I2C_MEMADD_SIZE_8BIT,
            data,
            sizeof(data),
            5U
        );
        if (status == HAL_OK) {
            // Ghep 8 byte cao voi 8 byte thap
            s.ax = (int16_t)(
                ((uint16_t)data[0] << 8) |
                (uint16_t)data[1]
            );

            s.ay = (int16_t)(
                ((uint16_t)data[2] << 8) |
                (uint16_t)data[3]
            );

            s.az = (int16_t)(
                ((uint16_t)data[4] << 8) |
                (uint16_t)data[5]
            );
            g_sensor_read_ok_count++;
            // Gui data raw s vao queue
            if (xQueueSend(queue_raw, &s, 0) != pdPASS) {
                g_raw_drop_count++;
            }
        } else {
            g_mpu_status = status;
            g_i2c_error_count++;
        }
        /* TODO 3: xQueueSend(queue_raw, &s, <timeout>) — pick and justify the
         *         timeout in PREDICTION.md question 3/4. What happens to this
         *         sample if the queue is full and the timeout expires? */
        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(SAMPLE_PERIOD_MS)
        );
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}
