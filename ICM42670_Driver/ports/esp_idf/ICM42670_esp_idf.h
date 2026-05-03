/**
 * @file ICM42670_esp_idf.h
 * @brief ESP-IDF transport adapter for the ICM-42670-P driver.
 */

#ifndef ICM42670_ESP_IDF_H
#define ICM42670_ESP_IDF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#define ICM42670_ESP_DEFAULT_I2C_SPEED_HZ 400000U
#define ICM42670_ESP_DEFAULT_SPI_SPEED_HZ 1000000

typedef struct {
  i2c_master_bus_handle_t bus_handle;
  i2c_master_dev_handle_t dev_handle;
  uint32_t timeout_ms;
  bool owns_bus;
  bool owns_device;
} ICM42670_ESP_I2CBus;

typedef struct {
  spi_host_device_t host_id;
  spi_device_handle_t dev_handle;
  uint32_t timeout_ms;
  bool owns_bus;
  bool owns_device;
} ICM42670_ESP_SPIBus;

esp_err_t ICM42670_ESP_I2C_Init(ICM42670_Config *config,
                                ICM42670_ESP_I2CBus *bus, int i2c_port,
                                gpio_num_t sda_pin, gpio_num_t scl_pin,
                                uint8_t device_addr,
                                uint32_t scl_speed_hz);

esp_err_t ICM42670_ESP_I2C_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_I2CBus *bus,
                                        i2c_master_bus_handle_t bus_handle,
                                        i2c_master_dev_handle_t dev_handle);

esp_err_t ICM42670_ESP_I2C_Deinit(ICM42670_ESP_I2CBus *bus);

esp_err_t ICM42670_ESP_I2C_Probe(ICM42670_ESP_I2CBus *bus,
                                 uint8_t device_addr);

int8_t ICM42670_ESP_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len);

int8_t ICM42670_ESP_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len);

esp_err_t ICM42670_ESP_SPI_Init(ICM42670_Config *config,
                                ICM42670_ESP_SPIBus *bus,
                                spi_host_device_t host_id, gpio_num_t miso_pin,
                                gpio_num_t mosi_pin, gpio_num_t sclk_pin,
                                gpio_num_t cs_pin, int clock_speed_hz);

esp_err_t ICM42670_ESP_SPI_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_SPIBus *bus,
                                        spi_host_device_t host_id,
                                        spi_device_handle_t dev_handle);

esp_err_t ICM42670_ESP_SPI_Deinit(ICM42670_ESP_SPIBus *bus);

int8_t ICM42670_ESP_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len);

int8_t ICM42670_ESP_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_ESP_IDF_H */
