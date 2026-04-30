/**
 *
 * @file        ICM42670_driver.c
 * @brief
 *               This file will contain the main logic for using the ICM42670
 * chip
 * @author Anthony / SleepPandas
 */

#include "ICM42670_driver.h"
#include <stdint.h>

static int16_t ICM42670_CombineBytes(uint8_t msb, uint8_t lsb) {
  return (int16_t)(((uint16_t)msb << 8U) | (uint16_t)lsb);
}

static ICM42670_AccelFS_t
ICM42670_NormalizeAccelFs(ICM42670_AccelFS_t accel_fs) {
  switch (accel_fs) {
  case ICM42670_ACCEL_FS_16G:
  case ICM42670_ACCEL_FS_8G:
  case ICM42670_ACCEL_FS_4G:
  case ICM42670_ACCEL_FS_2G:
    return accel_fs;
  default:
    return ICM42670_ACCEL_FS_16G;
  }
}

static ICM42670_GyroFS_t ICM42670_NormalizeGyroFs(ICM42670_GyroFS_t gyro_fs) {
  switch (gyro_fs) {
  case ICM42670_GYRO_FS_2000_DPS:
  case ICM42670_GYRO_FS_1000_DPS:
  case ICM42670_GYRO_FS_500_DPS:
  case ICM42670_GYRO_FS_250_DPS:
    return gyro_fs;
  default:
    return ICM42670_GYRO_FS_2000_DPS;
  }
}

static ICM42670_Odr_t ICM42670_NormalizeAccelOdr(ICM42670_Odr_t odr) {
  switch (odr) {
  case ICM42670_ODR_1600_HZ:
  case ICM42670_ODR_800_HZ:
  case ICM42670_ODR_400_HZ:
  case ICM42670_ODR_200_HZ:
  case ICM42670_ODR_100_HZ:
  case ICM42670_ODR_50_HZ:
  case ICM42670_ODR_25_HZ:
  case ICM42670_ODR_12_5_HZ:
  case ICM42670_ACCEL_ODR_6_25_HZ:
  case ICM42670_ACCEL_ODR_3_125_HZ:
  case ICM42670_ACCEL_ODR_1_5625_HZ:
    return odr;
  default:
    return ICM42670_ODR_100_HZ;
  }
}

static ICM42670_Odr_t ICM42670_NormalizeGyroOdr(ICM42670_Odr_t odr) {
  switch (odr) {
  case ICM42670_ODR_1600_HZ:
  case ICM42670_ODR_800_HZ:
  case ICM42670_ODR_400_HZ:
  case ICM42670_ODR_200_HZ:
  case ICM42670_ODR_100_HZ:
  case ICM42670_ODR_50_HZ:
  case ICM42670_ODR_25_HZ:
  case ICM42670_ODR_12_5_HZ:
    return odr;
  default:
    return ICM42670_ODR_100_HZ;
  }
}

static float ICM42670_AccelLsbPerG(ICM42670_AccelFS_t accel_fs) {
  switch (accel_fs) {
  case ICM42670_ACCEL_FS_2G:
    return 16384.0f;
  case ICM42670_ACCEL_FS_4G:
    return 8192.0f;
  case ICM42670_ACCEL_FS_8G:
    return 4096.0f;
  case ICM42670_ACCEL_FS_16G:
  default:
    return 2048.0f;
  }
}

static float ICM42670_GyroLsbPerDps(ICM42670_GyroFS_t gyro_fs) {
  switch (gyro_fs) {
  case ICM42670_GYRO_FS_250_DPS:
    return 131.0f;
  case ICM42670_GYRO_FS_500_DPS:
    return 65.5f;
  case ICM42670_GYRO_FS_1000_DPS:
    return 32.8f;
  case ICM42670_GYRO_FS_2000_DPS:
  default:
    return 16.4f;
  }
}

static ICM42670_Status_t
ICM42670_PowerStateToReg(ICM42670_PowerState_t state, uint8_t *pwr_mgmt0) {
  if (pwr_mgmt0 == 0) {
    return ICM42670_ERROR;
  }

  switch (state) {
  case ICM42670_POWER_SLEEP:
    *pwr_mgmt0 = ICM42670_PWR_GYRO_OFF | ICM42670_PWR_ACCEL_OFF;
    return ICM42670_OK;
  case ICM42670_POWER_STANDBY:
    *pwr_mgmt0 = ICM42670_PWR_GYRO_STANDBY | ICM42670_PWR_ACCEL_OFF;
    return ICM42670_OK;
  case ICM42670_POWER_ACCEL_LP:
    *pwr_mgmt0 = ICM42670_PWR_GYRO_OFF | ICM42670_PWR_ACCEL_LP;
    return ICM42670_OK;
  case ICM42670_POWER_ACCEL_LN:
    *pwr_mgmt0 = ICM42670_PWR_GYRO_OFF | ICM42670_PWR_ACCEL_LN;
    return ICM42670_OK;
  case ICM42670_POWER_GYRO_LN:
    *pwr_mgmt0 = ICM42670_PWR_GYRO_LN | ICM42670_PWR_ACCEL_OFF;
    return ICM42670_OK;
  case ICM42670_POWER_6AXIS_LN:
    *pwr_mgmt0 = ICM42670_PWR_ACCEL_GYRO_LN;
    return ICM42670_OK;
  default:
    return ICM42670_ERROR;
  }
}

ICM42670_Status_t ICM42670_SetPowerState(ICM42670_Config *config,
                                         ICM42670_PowerState_t state) {
  uint8_t pwr_mgmt0 = 0;

  if ((config == 0) || (config->write_reg == 0) || (config->delay_ms == 0)) {
    return ICM42670_ERROR;
  }

  if (ICM42670_PowerStateToReg(state, &pwr_mgmt0) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_PWR_MGMT0, &pwr_mgmt0,
                        1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(1);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_Init(ICM42670_Config *config) {
  uint8_t who_am_i = 0;
  uint8_t gyro_config = 0;
  uint8_t accel_config = 0;
  // Check for null pointers in config and required function pointers
  if ((config == 0) || (config->read_reg == 0) || (config->write_reg == 0) ||
      (config->delay_ms == 0)) {
    return ICM42670_ERROR;
  }
  // Normalize config values to valid ranges (e.g., if user passes in invalid
  // ODR or FS, set to default)

  config->accel_fs = ICM42670_NormalizeAccelFs(config->accel_fs);
  config->accel_odr = ICM42670_NormalizeAccelOdr(config->accel_odr);
  config->gyro_fs = ICM42670_NormalizeGyroFs(config->gyro_fs);
  config->gyro_odr = ICM42670_NormalizeGyroOdr(config->gyro_odr);

  // Check Who am I is valid
  if (config->read_reg(config->handle, ICM42670_REG_WHO_AM_I, &who_am_i, 1) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (who_am_i != ICM42670_WHO_AM_I_VALUE) {
    return ICM42670_ERROR;
  }

  gyro_config = (uint8_t)config->gyro_fs | (uint8_t)config->gyro_odr;
  accel_config = (uint8_t)config->accel_fs | (uint8_t)config->accel_odr;

  if (config->write_reg(config->handle, ICM42670_REG_GYRO_CONFIG0, &gyro_config,
                        1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_ACCEL_CONFIG0,
                        &accel_config, 1) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_SetPowerState(config, ICM42670_POWER_6AXIS_LN) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(50);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadAccelRaw(const ICM42670_Config *config,
                                        int16_t accel_raw[3]) {
  uint8_t raw[ICM42670_ACCEL_DATA_LEN] = {0};

  if ((config == 0) || (config->read_reg == 0) || (accel_raw == 0)) {
    return ICM42670_ERROR;
  }

  if (config->read_reg(config->handle, ICM42670_REG_ACCEL_DATA_X1, raw,
                       ICM42670_ACCEL_DATA_LEN) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  accel_raw[0] = ICM42670_CombineBytes(raw[0], raw[1]);
  accel_raw[1] = ICM42670_CombineBytes(raw[2], raw[3]);
  accel_raw[2] = ICM42670_CombineBytes(raw[4], raw[5]);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadGyroRaw(const ICM42670_Config *config,
                                       int16_t gyro_raw[3]) {
  uint8_t raw[ICM42670_GYRO_DATA_LEN] = {0};

  if ((config == 0) || (config->read_reg == 0) || (gyro_raw == 0)) {
    return ICM42670_ERROR;
  }

  if (config->read_reg(config->handle, ICM42670_REG_GYRO_DATA_X1, raw,
                       ICM42670_GYRO_DATA_LEN) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  gyro_raw[0] = ICM42670_CombineBytes(raw[0], raw[1]);
  gyro_raw[1] = ICM42670_CombineBytes(raw[2], raw[3]);
  gyro_raw[2] = ICM42670_CombineBytes(raw[4], raw[5]);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadTempRaw(const ICM42670_Config *config,
                                       int16_t *temp_raw) {
  uint8_t raw[ICM42670_TEMP_DATA_LEN] = {0};

  if ((config == 0) || (config->read_reg == 0) || (temp_raw == 0)) {
    return ICM42670_ERROR;
  }

  if (config->read_reg(config->handle, ICM42670_REG_TEMP_DATA1, raw,
                       ICM42670_TEMP_DATA_LEN) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *temp_raw = ICM42670_CombineBytes(raw[0], raw[1]);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadAccelG(const ICM42670_Config *config,
                                      ICM42670_Accel_t *accel) {
  int16_t accel_raw[3] = {0};
  float lsb_per_g = 0.0f;

  if ((config == 0) || (accel == 0)) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadAccelRaw(config, accel_raw) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  lsb_per_g = ICM42670_AccelLsbPerG(config->accel_fs);
  accel->x_g = (float)accel_raw[0] / lsb_per_g;
  accel->y_g = (float)accel_raw[1] / lsb_per_g;
  accel->z_g = (float)accel_raw[2] / lsb_per_g;
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadGyroDps(const ICM42670_Config *config,
                                       ICM42670_Gyro_t *gyro) {
  int16_t gyro_raw[3] = {0};
  float lsb_per_dps = 0.0f;

  if ((config == 0) || (gyro == 0)) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadGyroRaw(config, gyro_raw) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  lsb_per_dps = ICM42670_GyroLsbPerDps(config->gyro_fs);
  gyro->x_dps =
      (float)(gyro_raw[0] - config->gyro_offsets.x_raw_offset) / lsb_per_dps;
  gyro->y_dps =
      (float)(gyro_raw[1] - config->gyro_offsets.y_raw_offset) / lsb_per_dps;
  gyro->z_dps =
      (float)(gyro_raw[2] - config->gyro_offsets.z_raw_offset) / lsb_per_dps;
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadTempC(const ICM42670_Config *config,
                                     float *temp_c) {
  int16_t temp_raw = 0;

  if (temp_c == 0) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadTempRaw(config, &temp_raw) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *temp_c = ((float)temp_raw / 128.0f) + 25.0f;
  return ICM42670_OK;
}

/**
 * @brief Perform calibration of the ICM42670 IMU. Does not write to Cal
 * registers, but instead calculates and returns bias values for accel and gyro.
 * @note This is a simple calibration routine that assumes the device is
 * stationary and level during calibration.
 * @param config Pointer to the ICM42670 configuration structure.
 * @return Status of the calibration operation.
 */
ICM42670_Status_t ICM42670_Gyro_Calibration(ICM42670_Config *config) {
  int32_t gyro_bias[3] = {0};
  int16_t gyro_raw[3] = {0};

  // Collect multiple samples and average to find bias
  const int num_samples = 200;

  for (int i = 0; i < num_samples; i++) {
    if (ICM42670_ReadGyroRaw(config, gyro_raw) != ICM42670_OK) {
      return ICM42670_ERROR;
    }
    gyro_bias[0] += gyro_raw[0];
    gyro_bias[1] += gyro_raw[1];
    gyro_bias[2] += gyro_raw[2];
    config->delay_ms(10);
  }

  // find raw averages
  gyro_bias[0] = gyro_bias[0] / num_samples;
  gyro_bias[1] = gyro_bias[1] / num_samples;
  gyro_bias[2] = gyro_bias[2] / num_samples;

  config->gyro_offsets.x_raw_offset = (gyro_bias[0]);
  config->gyro_offsets.y_raw_offset = (gyro_bias[1]);
  config->gyro_offsets.z_raw_offset = (gyro_bias[2]);

  return ICM42670_OK;
}
