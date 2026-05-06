# Quick Start

This quick start covers the portable callback model first, then the two
included adapter paths: ESP-IDF and STM32 HAL. The driver core is plain C and
does not depend on either platform.

## What You Need To Provide

Fill an `ICM42670_Config` with sensor ranges, output data rates, and a small
transport layer.

| Config Field | Required | Purpose |
| --- | --- | --- |
| `read_reg` | Yes | Read one or more sensor registers |
| `write_reg` | Yes | Write one or more sensor registers |
| `delay_ms` | Yes | Blocking millisecond delay used during init and mode changes |
| `handle` | Yes | User pointer forwarded into `read_reg` and `write_reg` |
| `accel_odr` | Recommended | Accelerometer output data rate |
| `accel_fs` | Recommended | Accelerometer full-scale range |
| `gyro_odr` | Recommended | Gyroscope output data rate |
| `gyro_fs` | Recommended | Gyroscope full-scale range |
| `gyro_offsets` | Optional | Raw gyro offsets subtracted by `ICM42670_ReadGyroDps()` |

## 1. Use The Portable Callback Path

If your platform does not use one of the included adapters, wire the callbacks
directly.

```c
#include "ICM42670_driver.h"

static int8_t app_read_reg(void *handle, uint8_t reg, uint8_t *data,
                           uint16_t len) {
  /* Call your I2C, SPI, or I3C register-read function here. */
  return 0;
}

static int8_t app_write_reg(void *handle, uint8_t reg, const uint8_t *data,
                            uint16_t len) {
  /* Call your I2C, SPI, or I3C register-write function here. */
  return 0;
}

static void app_delay_ms(uint32_t ms) {
  /* Call your platform delay function here. */
}

ICM42670_Config imu = {
    .accel_odr = ICM42670_ODR_100_HZ,
    .accel_fs = ICM42670_ACCEL_FS_4G,
    .gyro_odr = ICM42670_ODR_100_HZ,
    .gyro_fs = ICM42670_GYRO_FS_500_DPS,
    .handle = &your_bus_state,
    .read_reg = app_read_reg,
    .write_reg = app_write_reg,
    .delay_ms = app_delay_ms,
};

if (ICM42670_Init(&imu) != ICM42670_OK) {
  /* Check wiring, bus mode, and WHO_AM_I. */
}
```

## 2. Read Accel, Gyro, And Temperature

```c
ICM42670_Accel_t accel = {0};
ICM42670_Gyro_t gyro = {0};
float temp_c = 0.0f;

if (ICM42670_ReadAccelG(&imu, &accel) == ICM42670_OK &&
    ICM42670_ReadGyroDps(&imu, &gyro) == ICM42670_OK &&
    ICM42670_ReadTempC(&imu, &temp_c) == ICM42670_OK) {
  /* Use accel.x_g, gyro.x_dps, and temp_c. */
}
```

Raw read helpers are also available:

```c
int16_t accel_raw[3] = {0};
int16_t gyro_raw[3] = {0};
int16_t temp_raw = 0;

ICM42670_ReadAccelRaw(&imu, accel_raw);
ICM42670_ReadGyroRaw(&imu, gyro_raw);
ICM42670_ReadTempRaw(&imu, &temp_raw);
```

## 3. ESP-IDF I2C Adapter

In a normal ESP-IDF app, add the component dependency:

```sh
idf.py add-dependency "sleepypandas/icm42670_driver^0.1.2"
idf.py reconfigure
```

Then use the adapter to create the ESP-IDF bus/device and populate the portable
driver config.

```c
#include "ICM42670_driver.h"
#include "ICM42670_esp_idf.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "ICM42670";

void app_main(void) {
  ICM42670_Config imu = {
      .accel_odr = ICM42670_ODR_100_HZ,
      .accel_fs = ICM42670_ACCEL_FS_4G,
      .gyro_odr = ICM42670_ODR_100_HZ,
      .gyro_fs = ICM42670_GYRO_FS_500_DPS,
  };
  ICM42670_ESP_I2CBus bus = {0};

  esp_err_t err = ICM42670_ESP_I2C_Init(&imu, &bus, I2C_NUM_0, GPIO_NUM_21,
                                        GPIO_NUM_22, 0x68, 400000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C adapter init failed: %s", esp_err_to_name(err));
    return;
  }

  if (ICM42670_Init(&imu) != ICM42670_OK) {
    ESP_LOGE(TAG, "IMU init failed. Check wiring and address.");
    (void)ICM42670_ESP_I2C_Deinit(&bus);
    return;
  }
}
```

If your application already owns the ESP-IDF I2C device handle, use
`ICM42670_ESP_I2C_AttachDevice()` instead of creating another bus/device.

## 4. STM32 HAL Adapter

For STM32 projects, initialize the CubeMX-generated peripherals first, then let
the adapter populate the same portable `ICM42670_Config`.

```c
#include "ICM42670_driver.h"
#include "ICM42670_stm32_hal.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

static ICM42670_Config imu = {
    .accel_odr = ICM42670_ODR_100_HZ,
    .accel_fs = ICM42670_ACCEL_FS_4G,
    .gyro_odr = ICM42670_ODR_100_HZ,
    .gyro_fs = ICM42670_GYRO_FS_500_DPS,
};
static ICM42670_STM32_SPIBus imu_spi = {0};

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();

  if (ICM42670_STM32_SPI_INIT(&imu, &imu_spi, &hspi1, IMU_CS_GPIO_Port,
                              IMU_CS_Pin) != ICM42670_OK) {
    Error_Handler();
  }

  if (ICM42670_Init(&imu) != ICM42670_OK) {
    Error_Handler();
  }

  while (1) {
    ICM42670_Accel_t accel = {0};
    ICM42670_ReadAccelG(&imu, &accel);
    HAL_Delay(100);
  }
}
```

STM32 I2C and I3C helpers follow the same pattern when the matching HAL module
is enabled.

## 5. Optional Features

Include only the optional module you need:

```c
#include "ICM42670_apex.h"
#include "ICM42670_fifo.h"
#include "ICM42670_fsync.h"
```

- Call `ICM42670_Init_Apex()` before enabling APEX pedometer, tilt, low-g,
  freefall, wake-on-motion, or significant-motion features.
- Use `ICM42670_Read_Apex()` when more than one APEX feature is enabled because
  the status registers are read-clear.
- Use `ICM42670_FIFO_Init()` for the default accel + gyro FIFO packet format.
- Use `ICM42670_Enable_Fsync()` when an external sync edge should be captured
  with timestamp data.

## 6. Bring-Up Checklist

- Confirm the device address or SPI chip-select wiring.
- Confirm `WHO_AM_I` reads `0x67`.
- Start at `100 Hz` and moderate ranges such as `4 g` and `500 dps`.
- For ESP-IDF I2C timeouts, re-check SDA/SCL pins, pullups, address, and bus
  ownership before changing driver code.
- For STM32 SPI, drive chip select high before the first transfer and verify
  the SPI mode expected by the sensor.

## Next

- Project summary: [Project Overview](@ref project_overview)
- Core API files: `ICM42670_driver.h`, `ICM42670_apex.h`,
  `ICM42670_fifo.h`, and `ICM42670_fsync.h`
- ESP-IDF example app:
  [examples/esp_idf/basic_i2c](https://github.com/SleepyPandas/ICM42670_Driver/tree/main/ICM42670_Driver/examples/esp_idf/basic_i2c)
