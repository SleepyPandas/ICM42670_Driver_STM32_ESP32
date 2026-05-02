/**
 * @file ICM42670_internal.c
 * @brief Private helpers shared by the portable ICM-42670-P driver modules.
 */

#include "ICM42670_internal.h"
#include <stddef.h>

#define ICM42670_MREG_DELAY_MS 1U

static ICM42670_Status_t
ICM42670_ValidateWriteConfig(const ICM42670_Config *config) {
  if ((config == NULL) || (config->write_reg == NULL)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

ICM42670_Status_t
ICM42670_ValidateConfig(const ICM42670_Config *config) {
  if ((config == NULL) || (config->read_reg == NULL) ||
      (config->write_reg == NULL)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

ICM42670_Status_t
ICM42670_ValidateMregConfig(const ICM42670_Config *config) {
  if ((ICM42670_ValidateConfig(config) != ICM42670_OK) ||
      (config->delay_ms == NULL)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_ReadReg(const ICM42670_Config *config,
                                   uint8_t reg_addr, uint8_t *data,
                                   uint16_t len) {
  if ((ICM42670_ValidateConfig(config) != ICM42670_OK) || (data == NULL) ||
      (len == 0U)) {
    return ICM42670_ERROR;
  }

  return (config->read_reg(config->handle, reg_addr, data, len) == ICM42670_OK)
             ? ICM42670_OK
             : ICM42670_ERROR;
}

ICM42670_Status_t ICM42670_WriteReg(const ICM42670_Config *config,
                                    uint8_t reg_addr, const uint8_t *data,
                                    uint16_t len) {
  if ((ICM42670_ValidateWriteConfig(config) != ICM42670_OK) ||
      (data == NULL) || (len == 0U)) {
    return ICM42670_ERROR;
  }

  return (config->write_reg(config->handle, reg_addr, data, len) ==
          ICM42670_OK)
             ? ICM42670_OK
             : ICM42670_ERROR;
}

ICM42670_Status_t ICM42670_WriteReg8(const ICM42670_Config *config,
                                     uint8_t reg_addr, uint8_t value) {
  return ICM42670_WriteReg(config, reg_addr, &value, 1U);
}

ICM42670_Status_t ICM42670_UpdateRegBits(const ICM42670_Config *config,
                                         uint8_t reg_addr, uint8_t mask,
                                         uint8_t field_value) {
  uint8_t value = 0U;

  if (ICM42670_ReadReg(config, reg_addr, &value, 1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  value = (uint8_t)((value & (uint8_t)~mask) | (field_value & mask));
  return ICM42670_WriteReg8(config, reg_addr, value);
}

ICM42670_Status_t
ICM42670_WaitForMclk(const ICM42670_Config *config) {
  uint8_t mclk_status = 0U;

  if (ICM42670_ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_MCLK_RDY, &mclk_status, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return ((mclk_status & ICM42670_MCLK_RDY_MASK) != 0U) ? ICM42670_OK
                                                        : ICM42670_BUSY;
}

ICM42670_Status_t ICM42670_ReadMreg1(const ICM42670_Config *config,
                                     uint8_t reg_addr, uint8_t *value) {
  ICM42670_Status_t status;

  if (value == NULL) {
    return ICM42670_ERROR;
  }

  status = ICM42670_WaitForMclk(config);
  if (status != ICM42670_OK) {
    return status;
  }

  if (ICM42670_WriteReg8(config, ICM42670_REG_BLK_SEL_R, ICM42670_MREG1) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_WriteReg8(config, ICM42670_REG_MADDR_R, reg_addr) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_MREG_DELAY_MS);

  if (ICM42670_ReadReg(config, ICM42670_REG_M_R, value, 1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_MREG_DELAY_MS);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_WriteMreg1(const ICM42670_Config *config,
                                      uint8_t reg_addr, uint8_t value) {
  ICM42670_Status_t status;

  status = ICM42670_WaitForMclk(config);
  if (status != ICM42670_OK) {
    return status;
  }

  if (ICM42670_WriteReg8(config, ICM42670_REG_BLK_SEL_W, ICM42670_MREG1) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_WriteReg8(config, ICM42670_REG_MADDR_W, reg_addr) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_WriteReg8(config, ICM42670_REG_M_W, value) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_MREG_DELAY_MS);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_UpdateMreg1Bits(const ICM42670_Config *config,
                                           uint8_t reg_addr, uint8_t mask,
                                           uint8_t field_value) {
  uint8_t value = 0U;
  ICM42670_Status_t status;

  status = ICM42670_ReadMreg1(config, reg_addr, &value);
  if (status != ICM42670_OK) {
    return status;
  }

  value = (uint8_t)((value & (uint8_t)~mask) | (field_value & mask));
  return ICM42670_WriteMreg1(config, reg_addr, value);
}

int16_t ICM42670_CombineBytes(uint8_t msb, uint8_t lsb) {
  return (int16_t)(((uint16_t)msb << 8U) | (uint16_t)lsb);
}
