/**
 * @file ICM42670_stm32_hal.c
 * @brief STM32 HAL SPI transport adapter for the ICM-42670-P driver.
 * This file implements the SPI/I2C read and write functions for the ICM-42670-P
 * driver, the goal is to provide a simple adapter layer that can be
 * used with the ICM-42670 driver on STM32(s) using the HAL library.
 */

#include "ICM42670_stm32_hal.h"
#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32h5xx_hal_i2c.h"
#endif
#ifdef HAL_I3C_MODULE_ENABLED
#include "stm32h5xx_hal_i3c.h"
#endif

#include <stddef.h>
#include <stdint.h>

#define ICM42670_STM32_SPI_READ_BIT 0x80U
#define ICM42670_STM32_SPI_MAX_TRANSFER_LEN 16U

#ifdef HAL_I2C_MODULE_ENABLED
#define ICM42670_STM32_I2C_MAX_TRANSFER_LEN 16U
#endif

#ifdef HAL_I3C_MODULE_ENABLED
#define ICM42670_STM32_I3C_MAX_TRANSFER_LEN 16U
#define ICM42670_STM32_I3C_DEFAULT_TIMEOUT_MS 1000U
#endif

static uint32_t ICM42670_STM32_SPI_Timeout(const ICM42670_STM32_SPIBus *bus) {
  return (bus->timeout_ms == 0U) ? HAL_MAX_DELAY : bus->timeout_ms;
}

#ifdef HAL_I2C_MODULE_ENABLED
static uint32_t ICM42670_STM32_I2C_Timeout(const ICM42670_STM32_I2CBus *bus) {
  return (bus->timeout_ms == 0U) ? HAL_MAX_DELAY : bus->timeout_ms;
}
#endif

#ifdef HAL_I3C_MODULE_ENABLED
static uint32_t ICM42670_STM32_I3C_Timeout(const ICM42670_STM32_I3CBus *bus) {
  return (bus->timeout_ms == 0U) ? HAL_MAX_DELAY : bus->timeout_ms;
}

static int8_t ICM42670_STM32_I3C_WaitReady(const ICM42670_STM32_I3CBus *bus) {
  uint32_t start = HAL_GetTick();
  uint32_t timeout = ICM42670_STM32_I3C_Timeout(bus);

  while (HAL_I3C_GetState(bus->hi3c) != HAL_I3C_STATE_READY) {
    if ((timeout != HAL_MAX_DELAY) && ((HAL_GetTick() - start) >= timeout)) {
      return ICM42670_ERROR;
    }
  }

  return ICM42670_OK;
}
#endif

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

#ifdef HAL_I3C_MODULE_ENABLED
ICM42670_Status_t ICM42670_STM32_I3C_INIT(ICM42670_Config *config,
                                          ICM42670_STM32_I3CBus *bus,
                                          I3C_HandleTypeDef *hi3c,
                                          uint8_t target_dynamic_addr) {
  if ((config == NULL) || (bus == NULL) || (hi3c == NULL)) {
    return ICM42670_ERROR;
  }

  bus->hi3c = hi3c;
  bus->target_dynamic_addr = target_dynamic_addr;
  bus->timeout_ms = ICM42670_STM32_I3C_DEFAULT_TIMEOUT_MS;

  config->handle = bus;
  config->read_reg = ICM42670_STM32_I3C_ReadReg;
  config->write_reg = ICM42670_STM32_I3C_WriteReg;
  config->delay_ms = HAL_Delay;

  return ICM42670_OK;
}

int8_t ICM42670_STM32_I3C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                  uint16_t len) {
  ICM42670_STM32_I3CBus *bus = (ICM42670_STM32_I3CBus *)handle;
  uint8_t tx_reg = reg_addr;
  uint32_t control[2] = {0};
  I3C_XferTypeDef xfer = {0};
  I3C_PrivateTypeDef descriptor[2] = {0};
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hi3c == NULL) || (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_I3C_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  descriptor[0].TargetAddr = bus->target_dynamic_addr;
  descriptor[0].TxBuf.pBuffer = &tx_reg;
  descriptor[0].TxBuf.Size = 1U;
  descriptor[0].Direction = HAL_I3C_DIRECTION_WRITE;

  descriptor[1].TargetAddr = bus->target_dynamic_addr;
  descriptor[1].RxBuf.pBuffer = data;
  descriptor[1].RxBuf.Size = len;
  descriptor[1].Direction = HAL_I3C_DIRECTION_READ;

  xfer.CtrlBuf.pBuffer = control;
  xfer.CtrlBuf.Size = 2U;
  xfer.TxBuf.pBuffer = &tx_reg;
  xfer.TxBuf.Size = 1U;
  xfer.RxBuf.pBuffer = data;
  xfer.RxBuf.Size = len;

  status = HAL_I3C_AddDescToFrame(bus->hi3c, NULL, descriptor, &xfer,
                                  (uint8_t)xfer.CtrlBuf.Size,
                                  I3C_PRIVATE_WITH_ARB_RESTART);
  if (status != HAL_OK) {
    return ICM42670_ERROR;
  }

  status = HAL_I3C_Ctrl_MultipleTransfer_IT(bus->hi3c, &xfer);
  if (status != HAL_OK) {
    return ICM42670_ERROR;
  }

  return ICM42670_STM32_I3C_WaitReady(bus);
}

int8_t ICM42670_STM32_I3C_WriteReg(void *handle, uint8_t reg_addr,
                                   const uint8_t *data, uint16_t len) {
  ICM42670_STM32_I3CBus *bus = (ICM42670_STM32_I3CBus *)handle;
  uint8_t tx[1U + ICM42670_STM32_I3C_MAX_TRANSFER_LEN] = {0};
  uint32_t control[1] = {0};
  I3C_XferTypeDef xfer = {0};
  I3C_PrivateTypeDef descriptor = {0};
  HAL_StatusTypeDef status;

  if ((bus == NULL) || (bus->hi3c == NULL) || (data == NULL) || (len == 0U) ||
      (len > ICM42670_STM32_I3C_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr;
  for (uint16_t i = 0; i < len; i++) {
    tx[i + 1U] = data[i];
  }

  descriptor.TargetAddr = bus->target_dynamic_addr;
  descriptor.TxBuf.pBuffer = tx;
  descriptor.TxBuf.Size = (uint32_t)len + 1U;
  descriptor.Direction = HAL_I3C_DIRECTION_WRITE;

  xfer.CtrlBuf.pBuffer = control;
  xfer.CtrlBuf.Size = 1U;
  xfer.TxBuf.pBuffer = tx;
  xfer.TxBuf.Size = (uint32_t)len + 1U;

  status = HAL_I3C_AddDescToFrame(bus->hi3c, NULL, &descriptor, &xfer,
                                  (uint8_t)xfer.CtrlBuf.Size,
                                  I3C_PRIVATE_WITH_ARB_STOP);
  if (status != HAL_OK) {
    return ICM42670_ERROR;
  }

  status = HAL_I3C_Ctrl_Transmit(bus->hi3c, &xfer,
                                 ICM42670_STM32_I3C_Timeout(bus));

  return (status == HAL_OK) ? ICM42670_OK : ICM42670_ERROR;
}
#endif /* HAL_I3C_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
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
#endif /* HAL_I2C_MODULE_ENABLED */

