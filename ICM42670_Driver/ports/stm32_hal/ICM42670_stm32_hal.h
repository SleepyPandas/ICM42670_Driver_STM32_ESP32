/**
 * @file ICM42670_stm32_hal.h
 * @brief STM32 HAL transport adapter for the ICM-42670-P driver.
 */

#ifndef ICM42670_STM32_HAL_H
#define ICM42670_STM32_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include "stm32h5xx_hal.h"

typedef struct {
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
  uint32_t timeout_ms;
} ICM42670_STM32_SPIBus;

ICM42670_Status_t ICM42670_STM32_SPI_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_SPIBus *bus,
                                          SPI_HandleTypeDef *hspi,
                                          GPIO_TypeDef *cs_port,
                                          uint16_t cs_pin);

int8_t ICM42670_STM32_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len);

int8_t ICM42670_STM32_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len);

typedef struct {
  // need SCLK and SDA which is in hi2c
  I2C_HandleTypeDef *hi2c;
  uint32_t timeout_ms;
} ICM42670_STM32_I2CBus;

ICM42670_Status_t ICM42670_STM32_I2C_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_I2CBus *bus,
                                          I2C_HandleTypeDef *hi2c);

int8_t ICM42670_STM32_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len);

int8_t ICM42670_STM32_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len,
                                   uint8_t device_addr);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_STM32_HAL_H */
