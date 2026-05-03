# ICM42670 ESP-IDF Component

Portable C driver for the ICM-42670-P IMU with a small ESP-IDF 5.x adapter for
I2C and SPI.

## Add To An ESP-IDF Project

After the component is published, add it from the ESP Component Registry:

```sh
idf.py add-dependency "<namespace>/icm42670^0.1.0"
```

For local testing before publishing, add this repository folder as an extra
component directory or use one of the included examples:

- `examples/esp_idf/basic_i2c`

## I2C Quick Start

```c
#include "ICM42670_driver.h"
#include "ICM42670_esp_idf.h"

ICM42670_Config imu = {
    .accel_odr = ICM42670_ODR_100_HZ,
    .accel_fs = ICM42670_ACCEL_FS_4G,
    .gyro_odr = ICM42670_ODR_100_HZ,
    .gyro_fs = ICM42670_GYRO_FS_500_DPS,
};
ICM42670_ESP_I2CBus bus = {0};

ESP_ERROR_CHECK(ICM42670_ESP_I2C_Init(&imu, &bus, I2C_NUM_0, GPIO_NUM_21,
                                      GPIO_NUM_22, 0x68, 400000));

if (ICM42670_Init(&imu) == ICM42670_OK) {
  ICM42670_Accel_t accel;
  ICM42670_Gyro_t gyro;
  float temp_c;

  ICM42670_ReadAccelG(&imu, &accel);
  ICM42670_ReadGyroDps(&imu, &gyro);
  ICM42670_ReadTempC(&imu, &temp_c);
}
```


## Example

The `examples/esp_idf/basic_i2c` app initializes the ESP-IDF I2C transport,
checks `WHO_AM_I`, runs a short gyro calibration, prints accel/gyro/temperature
data, and shows minimal APEX pedometer polling with `ICM42670_Read_Pedo()`.

The ESP adapter only fills the portable driver's transport callbacks. The normal
driver APIs remain the same for accel, gyro, temperature, and APEX.
