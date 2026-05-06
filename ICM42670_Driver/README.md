# ICM42670 ESP-IDF Component

Portable C driver for the ICM-42670-P IMU with a small ESP-IDF 5.x adapter for
I2C and SPI.

## Add To An ESP-IDF Project

After the component is published, add it from the ESP Component Registry:

```sh
idf.py add-dependency "sleepypandas/icm42670_driver^0.1.2"
```

For local testing before publishing, add this repository folder as an extra
component directory or use one of the included examples:

- `examples/esp_idf/basic_i2c`

If an older `0.1.0` or `0.1.1` release was already downloaded, update the
dependency to a new published version and reconfigure the project:

```sh
idf.py add-dependency "sleepypandas/icm42670_driver^0.1.2"
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
                                        GPIO_NUM_22, 0x68, 400000);
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

    vTaskDelay(pdMS_TO_TICKS(500));
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

## Release 0.1.2

Run these commands from this component directory in the ESP-IDF PowerShell
environment:

```powershell
cd C:\Users\AnthonyPC\Documents\GitHub\ICM42670_Driver\ICM42670_Driver
rg -n "version:|sleepypandas/icm42670_driver" README.md idf_component.yml
git diff --check
compote version
compote registry login --default-namespace sleepypandas
compote component upload --namespace sleepypandas --name icm42670_driver --dry-run
git status --short
git add README.md idf_component.yml examples/esp_idf/basic_i2c/CMakeLists.txt examples/esp_idf/basic_i2c/main/CMakeLists.txt examples/esp_idf/basic_i2c/main/main.c examples/esp_idf/basic_i2c/main/idf_component.yml
git commit -m "Release ESP-IDF component 0.1.2"
git tag v0.1.2
git push origin HEAD
git push origin v0.1.2
compote component upload --namespace sleepypandas --name icm42670_driver
```

After the upload finishes, verify the consumer command:

```powershell
idf.py add-dependency "sleepypandas/icm42670_driver^0.1.2"
idf.py reconfigure
```
