/**
 *
 * @file        ICM42670_driver.c
 * @brief
 *               This file will contain the main logic for using the ICM42670
 * chip
 * @author Anthony / SleepPandas
 */

#include "ICM42670_driver.h"
#include "ICM42670_registermap.h"

ICM42670_Status_t ICM42670_Init(const ICM42670_Config *config) {

  // Default Power Mode
  // Set accel config to default
  // Set gyro config to default
  // SET ODR to default
  // Delay for startup

// Read who AM I? 
    uint8_t who_am_i = 0;

    if (config->read_reg(config->handle, ICM42670_REG_WHO_AM_I, &who_am_i, 1) !=
        ICM42670_OK) {
      return ICM42670_ERROR;
    }
    if (who_am_i != ICM42670_WHO_AM_I_VALUE) {
      return ICM42670_ERROR;
    }

  uint8_t pwr_mgmt0 = ICM42670_PWR_ACCEL_GYRO_LN; // Low Noise Mode
  // needs to be on for at least 45ms after power-up, so delay before writing config
  config->delay_ms(50);

  if ((config == 0) || (config->write_reg == 0) || (config->delay_ms == 0)) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_PWR_MGMT0, &pwr_mgmt0,
                        1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  /*
   * Gyro low-noise mode has the longest startup time in the basic power-up
   * path, so wait before the first accel/gyro read.
   */
  config->delay_ms(50);

  // set Gyro accel ODR and FS
  uint8_t gyro_default = 0x06U;  // 2000 dps, 800 Hz
  uint8_t accel_default = 0x06U; // 16g, 800 Hz Low Noise mode

  if (config->write_reg(config->handle, ICM42670_REG_GYRO_CONFIG0,
                        &gyro_default, 1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }
  if (config->write_reg(config->handle, ICM42670_REG_ACCEL_CONFIG0,
                        &accel_default, 1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadAccelRaw(const ICM42670_Config *config,
                                        int16_t accel_raw[6]);

ICM42670_Status_t ICM42670_ReadGyroRaw(const ICM42670_Config *config,
                                       int16_t gyro_raw[6]);

ICM42670_Status_t ICM42670_ReadAccelG(const ICM42670_Config *config,
                                      ICM42670_Accel_t *accel);

ICM42670_Status_t ICM42670_ReadGyroDps(const ICM42670_Config *config,
                                       ICM42670_Gyro_t *gyro);
