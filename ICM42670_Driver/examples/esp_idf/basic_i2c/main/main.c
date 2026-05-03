#include "ICM42670_apex.h"
#include "ICM42670_driver.h"
#include "ICM42670_esp_idf.h"
#include "ICM42670_registermap.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>

#define ICM42670_EXAMPLE_I2C_PORT I2C_NUM_0
#define ICM42670_EXAMPLE_I2C_SDA GPIO_NUM_21
#define ICM42670_EXAMPLE_I2C_SCL GPIO_NUM_22
#define ICM42670_EXAMPLE_I2C_ADDR 0x68U
#define ICM42670_EXAMPLE_I2C_SPEED_HZ 100000U

#define ICM42670_EXAMPLE_START_ATTEMPTS 10U
#define ICM42670_EXAMPLE_START_RETRY_DELAY_MS 100U
#define ICM42670_EXAMPLE_READ_DELAY_MS 250U
#define ICM42670_EXAMPLE_APEX_READ_DELAY_MS 2000U
#define ICM42670_EXAMPLE_APEX_READ_RETRY_DELAY_MS 10U
#define ICM42670_EXAMPLE_APEX_START_ATTEMPTS 5U
#define ICM42670_EXAMPLE_APEX_RETRY_DELAY_MS 100U

static const char *TAG = "icm42670_i2c";

static const char *ICM42670_ExampleStatusName(ICM42670_Status_t status) {
  switch (status) {
  case ICM42670_OK:
    return "OK";
  case ICM42670_BUSY:
    return "BUSY";
  default:
    return "ERROR";
  }
}

static const char *
ICM42670_ExampleActivityName(ICM42670_ApexActivity_t activity) {
  switch (activity) {
  case ICM42670_APEX_ACTIVITY_WALK:
    return "walk";
  case ICM42670_APEX_ACTIVITY_RUN:
    return "run";
  default:
    return "unknown";
  }
}

static bool ICM42670_ExampleStartSensor(ICM42670_Config *imu_config) {
  for (uint32_t attempt = 1U; attempt <= ICM42670_EXAMPLE_START_ATTEMPTS;
       attempt++) {
    uint8_t who_am_i = 0U;

    if (imu_config->read_reg(imu_config->handle, ICM42670_REG_WHO_AM_I,
                             &who_am_i, 1U) != ICM42670_OK) {
      ESP_LOGW(TAG, "WHO_AM_I read failed, retry %lu/%u",
               (unsigned long)attempt, ICM42670_EXAMPLE_START_ATTEMPTS);
    } else if (who_am_i != ICM42670_WHO_AM_I_VALUE) {
      ESP_LOGW(TAG, "WHO_AM_I=0x%02X expected=0x%02X, retry %lu/%u",
               who_am_i, ICM42670_WHO_AM_I_VALUE, (unsigned long)attempt,
               ICM42670_EXAMPLE_START_ATTEMPTS);
    } else {
      ESP_LOGI(TAG, "WHO_AM_I=0x%02X expected=0x%02X", who_am_i,
               ICM42670_WHO_AM_I_VALUE);

      if (ICM42670_Init(imu_config) == ICM42670_OK) {
        return true;
      }

      ESP_LOGW(TAG, "ICM42670_Init failed, retry %lu/%u",
               (unsigned long)attempt, ICM42670_EXAMPLE_START_ATTEMPTS);
    }

    vTaskDelay(pdMS_TO_TICKS(ICM42670_EXAMPLE_START_RETRY_DELAY_MS));
  }

  return false;
}

static void ICM42670_ExampleScanI2c(ICM42670_ESP_I2CBus *bus) {
  bool found_device = false;

  ESP_LOGI(TAG, "I2C scan on SDA=%d SCL=%d", ICM42670_EXAMPLE_I2C_SDA,
           ICM42670_EXAMPLE_I2C_SCL);

  for (uint8_t addr = 0x08U; addr <= 0x77U; addr++) {
    if (ICM42670_ESP_I2C_Probe(bus, addr) == ESP_OK) {
      ESP_LOGI(TAG, "I2C device found at 0x%02X", addr);
      found_device = true;
    }
  }

  if (!found_device) {
    ESP_LOGW(TAG, "No I2C devices found");
  }

  ESP_LOGI(TAG, "Trying ICM42670 address 0x%02X",
           ICM42670_EXAMPLE_I2C_ADDR);
}

static bool ICM42670_ExampleEnableApex(ICM42670_Config *imu_config) {
  ICM42670_PedoConfig_t pedo_config = {
      .slow_walk_enable = 1U,
  };

  for (uint32_t attempt = 1U;
       attempt <= ICM42670_EXAMPLE_APEX_START_ATTEMPTS; attempt++) {
    ICM42670_Status_t status = ICM42670_Init_Apex(imu_config);

    if (status != ICM42670_OK) {
      ESP_LOGW(TAG, "APEX init failed: %s, retry %lu/%u",
               ICM42670_ExampleStatusName(status), (unsigned long)attempt,
               ICM42670_EXAMPLE_APEX_START_ATTEMPTS);
      vTaskDelay(pdMS_TO_TICKS(ICM42670_EXAMPLE_APEX_RETRY_DELAY_MS));
      continue;
    }

    status = ICM42670_Enable_Pedo(imu_config, &pedo_config);
    if (status == ICM42670_OK) {
      ESP_LOGI(TAG, "APEX pedometer enabled");
      return true;
    }

    ESP_LOGW(TAG, "APEX pedometer enable failed: %s, retry %lu/%u",
             ICM42670_ExampleStatusName(status), (unsigned long)attempt,
             ICM42670_EXAMPLE_APEX_START_ATTEMPTS);
    vTaskDelay(pdMS_TO_TICKS(ICM42670_EXAMPLE_APEX_RETRY_DELAY_MS));
  }

  ESP_LOGW(TAG, "APEX pedometer setup failed; continuing with normal reads");
  return false;
}

void app_main(void) {
  ICM42670_Config imu_config = {
      .accel_odr = ICM42670_ODR_100_HZ,
      .accel_fs = ICM42670_ACCEL_FS_4G,
      .gyro_odr = ICM42670_ODR_100_HZ,
      .gyro_fs = ICM42670_GYRO_FS_500_DPS,
  };
  ICM42670_ESP_I2CBus bus = {0};

  esp_err_t err = ICM42670_ESP_I2C_Init(
      &imu_config, &bus, ICM42670_EXAMPLE_I2C_PORT, ICM42670_EXAMPLE_I2C_SDA,
      ICM42670_EXAMPLE_I2C_SCL, ICM42670_EXAMPLE_I2C_ADDR,
      ICM42670_EXAMPLE_I2C_SPEED_HZ);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(err));
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(100));
  ICM42670_ExampleScanI2c(&bus);

  if (!ICM42670_ExampleStartSensor(&imu_config)) {
    ESP_LOGE(TAG, "ICM42670 start failed after retries");
    (void)ICM42670_ESP_I2C_Deinit(&bus);
    return;
  }

  ESP_LOGI(TAG, "Keep the board still: calibrating gyro for about 2 seconds");
  if (ICM42670_Gyro_Calibration(&imu_config) == ICM42670_OK) {
    ESP_LOGI(TAG, "gyro offsets raw x=%d y=%d z=%d",
             imu_config.gyro_offsets.x_raw_offset,
             imu_config.gyro_offsets.y_raw_offset,
             imu_config.gyro_offsets.z_raw_offset);
  } else {
    ESP_LOGW(TAG, "gyro calibration failed; continuing without offsets");
  }

  bool apex_enabled = ICM42670_ExampleEnableApex(&imu_config);
  TickType_t last_apex_read_tick = 0U;

  while (true) {
    ICM42670_Accel_t accel = {0};
    ICM42670_Gyro_t gyro = {0};
    float temp_c = 0.0f;

    if ((ICM42670_ReadAccelG(&imu_config, &accel) == ICM42670_OK) &&
        (ICM42670_ReadGyroDps(&imu_config, &gyro) == ICM42670_OK) &&
        (ICM42670_ReadTempC(&imu_config, &temp_c) == ICM42670_OK)) {
      ESP_LOGI(TAG,
               "accel[g] x=%.3f y=%.3f z=%.3f gyro[dps] x=%.2f y=%.2f "
               "z=%.2f temp=%.2f C",
               accel.x_g, accel.y_g, accel.z_g, gyro.x_dps, gyro.y_dps,
               gyro.z_dps, temp_c);
    } else {
      ESP_LOGW(TAG, "sensor read failed");
    }

    TickType_t now = xTaskGetTickCount();
    if (apex_enabled &&
        ((now - last_apex_read_tick) >=
         pdMS_TO_TICKS(ICM42670_EXAMPLE_APEX_READ_DELAY_MS))) {
      ICM42670_PedoData_t pedo = {0};
      last_apex_read_tick = now;

      ICM42670_Status_t pedo_status = ICM42670_Read_Pedo(&imu_config, &pedo);
      if (pedo_status != ICM42670_OK) {
        vTaskDelay(pdMS_TO_TICKS(ICM42670_EXAMPLE_APEX_READ_RETRY_DELAY_MS));
        pedo_status = ICM42670_Read_Pedo(&imu_config, &pedo);
      }

      if (pedo_status == ICM42670_OK) {
        ESP_LOGI(TAG, "apex pedometer steps=%u step=%u activity=%s",
                 (unsigned)pedo.step_count, (unsigned)pedo.step_detected,
                 ICM42670_ExampleActivityName(pedo.activity));
      } else {
        ESP_LOGW(TAG, "APEX pedometer read failed");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(ICM42670_EXAMPLE_READ_DELAY_MS));
  }
}
