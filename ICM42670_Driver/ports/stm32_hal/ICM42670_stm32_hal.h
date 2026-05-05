/**
 * @file ICM42670_stm32_hal.h
 * @brief STM32 HAL transport adapters for the ICM-42670-P driver.
 */

#ifndef ICM42670_STM32_HAL_H
#define ICM42670_STM32_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include "stm32h5xx_hal.h"

/**
 * @brief STM32 SPI bus state used by the portable driver callbacks.
 */
typedef struct {
  SPI_HandleTypeDef *hspi; /**< STM32 HAL SPI handle. */
  GPIO_TypeDef *cs_port; /**< Chip-select GPIO port. */
  uint16_t cs_pin; /**< Chip-select GPIO pin. */
  uint32_t timeout_ms; /**< HAL timeout in milliseconds. */
} ICM42670_STM32_SPIBus;

/**
 * @brief Configure an ICM42670_Config for STM32 HAL SPI access.
 *
 * This helper stores bus state, drives chip-select high, and installs the SPI
 * read/write callbacks plus HAL_Delay.
 *
 * @param config Driver configuration to populate.
 * @param bus SPI bus state owned by the caller.
 * @param hspi Initialized STM32 HAL SPI handle.
 * @param cs_port Chip-select GPIO port.
 * @param cs_pin Chip-select GPIO pin.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_STM32_SPI_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_SPIBus *bus,
                                          SPI_HandleTypeDef *hspi,
                                          GPIO_TypeDef *cs_port,
                                          uint16_t cs_pin);

/**
 * @brief STM32 SPI register-read callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_SPIBus.
 * @param reg_addr Register address to read from.
 * @param data Destination buffer.
 * @param len Number of bytes to read.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len);

/**
 * @brief STM32 SPI register-write callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_SPIBus.
 * @param reg_addr Register address to write to.
 * @param data Source buffer.
 * @param len Number of bytes to write.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len);

#ifdef HAL_I2C_MODULE_ENABLED
/**
 * @brief STM32 I2C bus state used by the portable driver callbacks.
 */
typedef struct {
  I2C_HandleTypeDef *hi2c; /**< STM32 HAL I2C handle. */
  uint8_t device_addr; /**< 7-bit I2C address, usually 0x68 or 0x69. */
  uint32_t timeout_ms; /**< HAL timeout in milliseconds. */
} ICM42670_STM32_I2CBus;

/**
 * @brief Configure an ICM42670_Config for STM32 HAL I2C access.
 *
 * @param config Driver configuration to populate.
 * @param bus I2C bus state owned by the caller.
 * @param hi2c Initialized STM32 HAL I2C handle.
 * @param device_addr 7-bit I2C device address.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_STM32_I2C_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_I2CBus *bus,
                                          I2C_HandleTypeDef *hi2c,
                                          uint8_t device_addr);

/**
 * @brief STM32 I2C register-read callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_I2CBus.
 * @param reg_addr Register address to read from.
 * @param data Destination buffer.
 * @param len Number of bytes to read.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len);

/**
 * @brief STM32 I2C register-write callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_I2CBus.
 * @param reg_addr Register address to write to.
 * @param data Source buffer.
 * @param len Number of bytes to write.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len);
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_I3C_MODULE_ENABLED
/**
 * @brief STM32 I3C bus state used by the portable driver callbacks.
 */
typedef struct {
  I3C_HandleTypeDef *hi3c; /**< STM32 HAL I3C handle. */
  uint8_t target_dynamic_addr; /**< 7-bit dynamic address assigned to target. */
  uint32_t timeout_ms; /**< HAL timeout in milliseconds. */
} ICM42670_STM32_I3CBus;

/**
 * @brief Configure an ICM42670_Config for STM32 HAL I3C access.
 *
 * @param config Driver configuration to populate.
 * @param bus I3C bus state owned by the caller.
 * @param hi3c Initialized STM32 HAL I3C handle.
 * @param target_dynamic_addr 7-bit target dynamic address.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_STM32_I3C_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_I3CBus *bus,
                                          I3C_HandleTypeDef *hi3c,
                                          uint8_t target_dynamic_addr);

/**
 * @brief STM32 I3C register-read callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_I3CBus.
 * @param reg_addr Register address to read from.
 * @param data Destination buffer.
 * @param len Number of bytes to read.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_I3C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len);

/**
 * @brief STM32 I3C register-write callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_STM32_I3CBus.
 * @param reg_addr Register address to write to.
 * @param data Source buffer.
 * @param len Number of bytes to write.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_STM32_I3C_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len);
#endif /* HAL_I3C_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_STM32_HAL_H */
