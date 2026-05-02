/**
 * @file ICM42670_internal.h
 * @brief Private helpers shared by the portable ICM-42670-P driver modules.
 */

#ifndef ICM42670_INTERNAL_H
#define ICM42670_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include <stdint.h>

ICM42670_Status_t
ICM42670_ValidateConfig(const ICM42670_Config *config);

ICM42670_Status_t
ICM42670_ValidateMregConfig(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_ReadReg(const ICM42670_Config *config,
                                   uint8_t reg_addr, uint8_t *data,
                                   uint16_t len);

ICM42670_Status_t ICM42670_WriteReg(const ICM42670_Config *config,
                                    uint8_t reg_addr, const uint8_t *data,
                                    uint16_t len);

ICM42670_Status_t ICM42670_WriteReg8(const ICM42670_Config *config,
                                     uint8_t reg_addr, uint8_t value);

ICM42670_Status_t ICM42670_UpdateRegBits(const ICM42670_Config *config,
                                         uint8_t reg_addr, uint8_t mask,
                                         uint8_t field_value);

ICM42670_Status_t
ICM42670_WaitForMclk(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_ReadMreg1(const ICM42670_Config *config,
                                     uint8_t reg_addr, uint8_t *value);

ICM42670_Status_t ICM42670_WriteMreg1(const ICM42670_Config *config,
                                      uint8_t reg_addr, uint8_t value);

ICM42670_Status_t ICM42670_UpdateMreg1Bits(const ICM42670_Config *config,
                                           uint8_t reg_addr, uint8_t mask,
                                           uint8_t field_value);

int16_t ICM42670_CombineBytes(uint8_t msb, uint8_t lsb);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_INTERNAL_H */
