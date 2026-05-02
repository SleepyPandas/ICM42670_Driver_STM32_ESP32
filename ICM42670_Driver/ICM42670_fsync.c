/**
 * @file ICM42670_fsync.c
 * @brief FSYNC configuration and polling helpers for the ICM-42670-P.
 */

#include "ICM42670_fsync.h"
#include <stddef.h>

#define ICM42670_FSYNC_MREG_DELAY_MS 1U

static ICM42670_Status_t ValidateConfig(const ICM42670_Config *config) {
  if ((config == NULL) || (config->read_reg == NULL) ||
      (config->write_reg == NULL)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

static ICM42670_Status_t ValidateMregConfig(const ICM42670_Config *config) {
  if ((ValidateConfig(config) != ICM42670_OK) || (config->delay_ms == NULL)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

static ICM42670_Status_t ReadReg(const ICM42670_Config *config,
                                 uint8_t reg_addr, uint8_t *data,
                                 uint16_t len) {
  if ((ValidateConfig(config) != ICM42670_OK) || (data == NULL) ||
      (len == 0U)) {
    return ICM42670_ERROR;
  }

  return (config->read_reg(config->handle, reg_addr, data, len) ==
          ICM42670_OK)
             ? ICM42670_OK
             : ICM42670_ERROR;
}

static ICM42670_Status_t WriteReg(const ICM42670_Config *config,
                                  uint8_t reg_addr, uint8_t value) {
  if (ValidateConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return (config->write_reg(config->handle, reg_addr, &value, 1U) ==
          ICM42670_OK)
             ? ICM42670_OK
             : ICM42670_ERROR;
}

static ICM42670_Status_t WaitForMclk(const ICM42670_Config *config) {
  uint8_t mclk_status = 0U;

  if (ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ReadReg(config, ICM42670_REG_MCLK_RDY, &mclk_status, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return ((mclk_status & ICM42670_MCLK_RDY_MASK) != 0U) ? ICM42670_OK
                                                        : ICM42670_BUSY;
}

static ICM42670_Status_t ReadMreg1(const ICM42670_Config *config,
                                   uint8_t reg_addr, uint8_t *value) {
  ICM42670_Status_t status;

  if (value == NULL) {
    return ICM42670_ERROR;
  }

  status = WaitForMclk(config);
  if (status != ICM42670_OK) {
    return status;
  }

  if (WriteReg(config, ICM42670_REG_BLK_SEL_R, ICM42670_MREG1) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (WriteReg(config, ICM42670_REG_MADDR_R, reg_addr) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_FSYNC_MREG_DELAY_MS);

  if (ReadReg(config, ICM42670_REG_M_R, value, 1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_FSYNC_MREG_DELAY_MS);
  return ICM42670_OK;
}

static ICM42670_Status_t WriteMreg1(const ICM42670_Config *config,
                                    uint8_t reg_addr, uint8_t value) {
  ICM42670_Status_t status;

  status = WaitForMclk(config);
  if (status != ICM42670_OK) {
    return status;
  }

  if (WriteReg(config, ICM42670_REG_BLK_SEL_W, ICM42670_MREG1) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (WriteReg(config, ICM42670_REG_MADDR_W, reg_addr) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (WriteReg(config, ICM42670_REG_M_W, value) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_FSYNC_MREG_DELAY_MS);
  return ICM42670_OK;
}

static ICM42670_Status_t UpdateMreg1Bits(const ICM42670_Config *config,
                                         uint8_t reg_addr, uint8_t mask,
                                         uint8_t field_value) {
  uint8_t value = 0U;
  ICM42670_Status_t status;

  status = ReadMreg1(config, reg_addr, &value);
  if (status != ICM42670_OK) {
    return status;
  }

  value = (uint8_t)((value & (uint8_t)~mask) | (field_value & mask));
  return WriteMreg1(config, reg_addr, value);
}

static ICM42670_Status_t NormalizeFsyncConfig(
    const ICM42670_FsyncConfig_t *input, ICM42670_FsyncConfig_t *output) {
  if (output == NULL) {
    return ICM42670_ERROR;
  }

  output->polarity = ICM42670_FSYNC_RISING_EDGE;
  output->tag = ICM42670_FSYNC_TAG_NONE;
  output->clear_mode = ICM42670_FSYNC_CLEAR_ON_SENSOR_UPDATE;
  output->interrupt_pin = ICM42670_FSYNC_INT_NONE;

  if (input == NULL) {
    return ICM42670_OK;
  }

  if ((input->polarity != ICM42670_FSYNC_RISING_EDGE) &&
      (input->polarity != ICM42670_FSYNC_FALLING_EDGE)) {
    return ICM42670_ERROR;
  }

  if (input->tag > ICM42670_FSYNC_TAG_ACCEL_Z) {
    return ICM42670_ERROR;
  }

  if ((input->clear_mode != ICM42670_FSYNC_CLEAR_ON_SENSOR_UPDATE) &&
      (input->clear_mode != ICM42670_FSYNC_CLEAR_ON_TAGGED_LSB_READ)) {
    return ICM42670_ERROR;
  }

  if ((input->interrupt_pin != ICM42670_FSYNC_INT_NONE) &&
      (input->interrupt_pin != ICM42670_FSYNC_INT1) &&
      (input->interrupt_pin != ICM42670_FSYNC_INT2)) {
    return ICM42670_ERROR;
  }

  if ((input->tag == ICM42670_FSYNC_TAG_NONE) &&
      (input->clear_mode == ICM42670_FSYNC_CLEAR_ON_TAGGED_LSB_READ)) {
    return ICM42670_ERROR;
  }

  *output = *input;
  return ICM42670_OK;
}

static ICM42670_Status_t RouteFsyncInterrupt(
    const ICM42670_Config *config, ICM42670_FsyncInterrupt_t interrupt_pin) {
  ICM42670_Status_t status;
  uint8_t int1_value = 0U;
  uint8_t int2_value = 0U;

  status = ReadReg(config, ICM42670_REG_INT_SOURCE0, &int1_value, 1U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ReadReg(config, ICM42670_REG_INT_SOURCE3, &int2_value, 1U);
  if (status != ICM42670_OK) {
    return status;
  }

  int1_value =
      (uint8_t)(int1_value & (uint8_t)~ICM42670_INT_SOURCE0_FSYNC_INT1_EN);
  int2_value =
      (uint8_t)(int2_value & (uint8_t)~ICM42670_INT_SOURCE3_FSYNC_INT2_EN);

  if (interrupt_pin == ICM42670_FSYNC_INT1) {
    int1_value = (uint8_t)(int1_value | ICM42670_INT_SOURCE0_FSYNC_INT1_EN);
  } else if (interrupt_pin == ICM42670_FSYNC_INT2) {
    int2_value = (uint8_t)(int2_value | ICM42670_INT_SOURCE3_FSYNC_INT2_EN);
  }

  status = WriteReg(config, ICM42670_REG_INT_SOURCE0, int1_value);
  if (status != ICM42670_OK) {
    return status;
  }

  return WriteReg(config, ICM42670_REG_INT_SOURCE3, int2_value);
}

ICM42670_Status_t
ICM42670_Enable_Fsync(const ICM42670_Config *config,
                      const ICM42670_FsyncConfig_t *fsync_config) {
  ICM42670_FsyncConfig_t active_config;
  ICM42670_Status_t status;
  uint8_t fsync_value = 0U;

  if (ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (NormalizeFsyncConfig(fsync_config, &active_config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_TMST_CONFIG1,
                           (uint8_t)(ICM42670_TMST_CONFIG1_RES_16US |
                                     ICM42670_TMST_CONFIG1_FSYNC_EN |
                                     ICM42670_TMST_CONFIG1_TMST_EN),
                           (uint8_t)(ICM42670_TMST_CONFIG1_FSYNC_EN |
                                     ICM42670_TMST_CONFIG1_TMST_EN));
  if (status != ICM42670_OK) {
    return status;
  }

  fsync_value =
      (uint8_t)(((uint8_t)active_config.tag
                 << ICM42670_FSYNC_CONFIG_UI_SEL_SHIFT) &
                ICM42670_FSYNC_CONFIG_UI_SEL_MASK);

  if (active_config.clear_mode == ICM42670_FSYNC_CLEAR_ON_TAGGED_LSB_READ) {
    fsync_value =
        (uint8_t)(fsync_value | ICM42670_FSYNC_CONFIG_UI_FLAG_CLEAR_SEL);
  }

  if (active_config.polarity == ICM42670_FSYNC_FALLING_EDGE) {
    fsync_value =
        (uint8_t)(fsync_value | ICM42670_FSYNC_CONFIG_POLARITY_FALLING);
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_FSYNC_CONFIG,
                           (uint8_t)(ICM42670_FSYNC_CONFIG_UI_SEL_MASK |
                                     ICM42670_FSYNC_CONFIG_UI_FLAG_CLEAR_SEL |
                                     ICM42670_FSYNC_CONFIG_POLARITY_FALLING),
                           fsync_value);
  if (status != ICM42670_OK) {
    return status;
  }

  return RouteFsyncInterrupt(config, active_config.interrupt_pin);
}

ICM42670_Status_t ICM42670_Disable_Fsync(const ICM42670_Config *config) {
  ICM42670_Status_t status;

  if (ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_FSYNC_CONFIG,
                           (uint8_t)(ICM42670_FSYNC_CONFIG_UI_SEL_MASK |
                                     ICM42670_FSYNC_CONFIG_UI_FLAG_CLEAR_SEL |
                                     ICM42670_FSYNC_CONFIG_POLARITY_FALLING),
                           0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_TMST_CONFIG1,
                           ICM42670_TMST_CONFIG1_FSYNC_EN, 0U);
  if (status != ICM42670_OK) {
    return status;
  }

  return RouteFsyncInterrupt(config, ICM42670_FSYNC_INT_NONE);
}

ICM42670_Status_t ICM42670_Read_Fsync(const ICM42670_Config *config,
                                      ICM42670_FsyncData_t *fsync_data) {
  uint8_t raw[2] = {0U};

  if ((ValidateConfig(config) != ICM42670_OK) || (fsync_data == NULL)) {
    return ICM42670_ERROR;
  }

  fsync_data->event_detected = 0U;
  fsync_data->raw_status = 0U;
  fsync_data->timestamp_delta_us = 0U;

  if (ReadReg(config, ICM42670_REG_INT_STATUS, &fsync_data->raw_status, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ReadReg(config, ICM42670_REG_TMST_FSYNCH, raw, 2U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  fsync_data->event_detected =
      ((fsync_data->raw_status & ICM42670_INT_STATUS_FSYNC) != 0U) ? 1U : 0U;
  fsync_data->timestamp_delta_us =
      (uint16_t)(((uint16_t)raw[0] << 8U) | (uint16_t)raw[1]);

  return ICM42670_OK;
}
