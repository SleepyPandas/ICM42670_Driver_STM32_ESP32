/**
 * @file ICM42670_stm32_hal.c
 * @brief STM32 HAL SPI transport adapter for the ICM-42670-P driver.
 * This file implements the SPI/I2C read and write functions for the ICM-42670-P
 * driver, the goal is to provide a simple adapter layer that can be
 * used with the ICM-42670 driver on STM32(s) using the HAL library.
 */

#include "ICM42670_stm32_hal.h"
#include "stm32h5xx_hal_i2c.h"

#include <stddef.h>
#include <stdint.h>

#define ICM42670_STM32_SPI_READ_BIT 0x80U
#define ICM42670_STM32_SPI_MAX_TRANSFER_LEN 16U

#define ICM42670_STM32_I2C_MAX_TRANSFER_LEN 16U

static uint32_t ICM42670_STM32_SPI_Timeout(const ICM42670_STM32_SPIBus *bus) {
  return (bus->timeout_ms == 0U) ? HAL_MAX_DELAY : bus->timeout_ms;
}

static uint32_t ICM42670_STM32_I2C_Timeout(const ICM42670_STM32_I2CBus *bus) {
  return (bus->timeout_ms == 0U) ? HAL_MAX_DELAY : bus->timeout_ms;
}

static void ICM42670_STM32_SPI_Select(const ICM42670_STM32_SPIBus *bus) {
  HAL_GPIO_WritePin(bus->cs_port, bus->cs_pin, GPIO_PIN_RESET);
}

static void ICM42670_STM32_SPI_Deselect(const ICM42670_STM32_SPIBus *bus) {
  HAL_GPIO_WritePin(bus->cs_port, bus->cs_pin, GPIO_PIN_SET);
}

ICM42670_Status_t ICM42670_STM32_SPI_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_SPIBus *bus,
                                          SPI_HandleTypeDef *hspi,
                                          GPIO_TypeDef *cs_port,
                                          uint16_t cs_pin) {
  if ((config == NULL) || (bus == NULL) || (hspi == NULL) ||
      (cs_port == NULL)) {
    return ICM42670_ERROR;
  }

  bus->hspi = hspi;
  bus->cs_port = cs_port;
  bus->cs_pin = cs_pin;
  bus->timeout_ms = HAL_MAX_DELAY;

  HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);

  config->handle = bus;
  config->read_reg = ICM42670_STM32_SPI_ReadReg;
  config->write_reg = ICM42670_STM32_SPI_WriteReg;
  config->delay_ms = HAL_Delay;

  return ICM42670_OK;
}

int8_t ICM42670_STM32_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len) {
  ICM42670_STM32_SPIBus *bus = (ICM42670_STM32_SPIBus *)handle;
  uint8_t tx[1U + ICM42670_STM32_SPI_MAX_TRANSFER_LEN] = {0};
  uint8_t rx[1U + ICM42670_STM32_SPI_MAX_TRANSFER_LEN] = {0};
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hspi == NULL) || (bus->cs_port == NULL) ||
      (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_SPI_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr | ICM42670_STM32_SPI_READ_BIT;

  ICM42670_STM32_SPI_Select(bus);
  status = HAL_SPI_TransmitReceive(bus->hspi, tx, rx, (uint16_t)(len + 1U),
                                   ICM42670_STM32_SPI_Timeout(bus));
  ICM42670_STM32_SPI_Deselect(bus);

  if (status != HAL_OK) {
    return ICM42670_ERROR;
  }

  for (uint16_t i = 0; i < len; i++) {
    data[i] = rx[i + 1U];
  }

  return ICM42670_OK;
}

int8_t ICM42670_STM32_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len) {
  ICM42670_STM32_SPIBus *bus = (ICM42670_STM32_SPIBus *)handle;
  uint8_t tx[1U + ICM42670_STM32_SPI_MAX_TRANSFER_LEN] = {0};
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hspi == NULL) || (bus->cs_port == NULL) ||
      (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_SPI_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr & (uint8_t)~ICM42670_STM32_SPI_READ_BIT;
  for (uint16_t i = 0; i < len; i++) {
    tx[i + 1U] = data[i];
  }

  ICM42670_STM32_SPI_Select(bus);
  status = HAL_SPI_Transmit(bus->hspi, tx, (uint16_t)(len + 1U),
                            ICM42670_STM32_SPI_Timeout(bus));
  ICM42670_STM32_SPI_Deselect(bus);

  return (status == HAL_OK) ? ICM42670_OK : ICM42670_ERROR;
}

ICM42670_Status_t ICM42670_STM32_I2C_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_I2CBus *bus,
                                          I2C_HandleTypeDef *hi2c,
                                          uint8_t device_addr) {
  if ((config == NULL) || (bus == NULL) || (hi2c == NULL)) {
    return ICM42670_ERROR;
  }

  bus->hi2c = hi2c;
  bus->device_addr = device_addr;
  bus->timeout_ms = HAL_MAX_DELAY;

  config->handle = bus;
  config->read_reg = ICM42670_STM32_I2C_ReadReg;
  config->write_reg = ICM42670_STM32_I2C_WriteReg;
  config->delay_ms = HAL_Delay;

  return ICM42670_OK;
}

int8_t ICM42670_STM32_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len) {
  ICM42670_STM32_I2CBus *bus = (ICM42670_STM32_I2CBus *)handle;
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hi2c == NULL) || (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_I2C_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  status = HAL_I2C_Mem_Read(bus->hi2c, (uint16_t)(bus->device_addr << 1U),
                            reg_addr, I2C_MEMADD_SIZE_8BIT, data, len,
                            ICM42670_STM32_I2C_Timeout(bus));

  return (status == HAL_OK) ? ICM42670_OK : ICM42670_ERROR;
}

int8_t ICM42670_STM32_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len) {
  ICM42670_STM32_I2CBus *bus = (ICM42670_STM32_I2CBus *)handle;
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hi2c == NULL) || (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_I2C_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  status = HAL_I2C_Mem_Write(bus->hi2c, (uint16_t)(bus->device_addr << 1U),
                             reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data,
                             len, ICM42670_STM32_I2C_Timeout(bus));

  return (status == HAL_OK) ? ICM42670_OK : ICM42670_ERROR;
}

