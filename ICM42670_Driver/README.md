# ICM42670 ESP-IDF Component

Portable C driver for the ICM-42670-P IMU with a small ESP-IDF 5.x adapter for
I2C and SPI.

## Add To An ESP-IDF Project

```sh
idf.py add-dependency "sleepypandas/icm42670_driver^1.0.0"
idf.py reconfigure
```

The dependency belongs in the consuming ESP-IDF app, usually in
`main/idf_component.yml`. Run `idf.py reconfigure` after adding or changing this
dependency so ESP-IDF refreshes `managed_components` and `dependencies.lock`.

## I2C Quick Start

```c
#include "ICM42670_driver.h"
#include "ICM42670_esp_idf.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ICM42670";

void app_main(void) {
  ICM42670_Config imu = {
      .accel_odr = ICM42670_ODR_800_HZ,
      .accel_fs = ICM42670_ACCEL_FS_4G,
      .gyro_odr = ICM42670_ODR_800_HZ,
      .gyro_fs = ICM42670_GYRO_FS_500_DPS,
  };
  ICM42670_ESP_I2CBus bus = {0};

  esp_err_t err = ICM42670_ESP_I2C_Init(&imu, &bus, I2C_NUM_0, GPIO_NUM_21,
                                        GPIO_NUM_22, 0x68, 100000);
  if (err == ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG,
             "I2C init failed: port or GPIO pins are already in use. Deinit "
             "the owner, use different pins, or attach to the existing bus.");
    return;
  } else if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(err));
    return;
  }

  if (ICM42670_Init(&imu) != ICM42670_OK) {
    ESP_LOGE(TAG, "ICM42670 init failed. Check wiring and I2C address.");
    (void)ICM42670_ESP_I2C_Deinit(&bus);
    return;
  }

  while (true) {
    ICM42670_Accel_t accel = {0};
    ICM42670_Gyro_t gyro = {0};
    float temp_c = 0.0f;

    ICM42670_ReadAccelG(&imu, &accel);
    ICM42670_ReadGyroDps(&imu, &gyro);
    ICM42670_ReadTempC(&imu, &temp_c);

    ESP_LOGI(TAG,
             "Accel: X=%.2f G, Y=%.2f G, Z=%.2f G | Gyro: X=%.2f DPS, "
             "Y=%.2f DPS, Z=%.2f DPS | Temperature: %.2f C",
             accel.x_g, accel.y_g, accel.z_g, gyro.x_dps, gyro.y_dps,
             gyro.z_dps, temp_c);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
```

If the log shows `I2C software timeout`, the ESP adapter resets the I2C master
bus once and retries the transfer. If the log also shows warnings such as
`GPIO 21 is not usable` or `GPIO 22 is not usable`, that is an init-time board
or ownership problem, not a normal sensor read failure. Check that those pins are
valid for your ESP board and that another driver or earlier init path has not
already claimed the same I2C port/pins. If your application creates the ESP-IDF
I2C bus itself, use `ICM42670_ESP_I2C_AttachDevice()` instead of calling
`ICM42670_ESP_I2C_Init()` a second time.


## Example

The README quick start above is intentionally the smallest copy-paste path for a
new ESP-IDF app. The `examples/esp_idf/basic_i2c` app is different: it is a
fuller board bring-up example with extra checks, retry behavior, gyro
calibration, and APEX pedometer polling.

The ESP adapter only fills the portable driver's transport callbacks. The normal
driver APIs remain the same for accel, gyro, temperature, and APEX.
