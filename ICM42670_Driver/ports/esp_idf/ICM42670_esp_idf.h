/**
 * @file ICM42670_esp_idf.h
 * @brief ESP-IDF I2C and SPI transport adapters for the ICM-42670-P driver.
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

#define ICM42670_ESP_DEFAULT_I2C_SPEED_HZ 400000U /**< Default I2C bus speed. */
#define ICM42670_ESP_DEFAULT_SPI_SPEED_HZ 1000000 /**< Default SPI clock speed. */

/**
 * @brief ESP-IDF I2C bus state used by the portable driver callbacks.
 */
typedef struct {
  i2c_master_bus_handle_t bus_handle; /**< ESP-IDF I2C master bus handle. */
  i2c_master_dev_handle_t dev_handle; /**< ESP-IDF I2C device handle. */
  uint32_t timeout_ms; /**< Transaction timeout in milliseconds. */
  bool owns_bus; /**< True when this adapter created the bus. */
  bool owns_device; /**< True when this adapter created the device. */
} ICM42670_ESP_I2CBus;

/**
 * @brief ESP-IDF SPI bus state used by the portable driver callbacks.
 */
typedef struct {
  spi_host_device_t host_id; /**< ESP-IDF SPI host. */
  spi_device_handle_t dev_handle; /**< ESP-IDF SPI device handle. */
  uint32_t timeout_ms; /**< Reserved transaction timeout in milliseconds. */
  bool owns_bus; /**< True when this adapter initialized the SPI bus. */
  bool owns_device; /**< True when this adapter added the SPI device. */
} ICM42670_ESP_SPIBus;

/**
 * @brief Create an ESP-IDF I2C bus/device and populate ICM42670_Config.
 *
 * The adapter owns the created bus and device until ICM42670_ESP_I2C_Deinit()
 * is called.
 *
 * @param config Driver configuration to populate.
 * @param bus I2C bus state owned by the caller.
 * @param i2c_port ESP-IDF I2C port number.
 * @param sda_pin SDA GPIO.
 * @param scl_pin SCL GPIO.
 * @param device_addr 7-bit I2C device address.
 * @param scl_speed_hz I2C clock speed, or 0 for the default.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ICM42670_ESP_I2C_Init(ICM42670_Config *config,
                                ICM42670_ESP_I2CBus *bus, int i2c_port,
                                gpio_num_t sda_pin, gpio_num_t scl_pin,
                                uint8_t device_addr,
                                uint32_t scl_speed_hz);

/**
 * @brief Attach an existing ESP-IDF I2C device to ICM42670_Config.
 *
 * The adapter does not own attached handles and will not remove them during
 * deinit.
 *
 * @param config Driver configuration to populate.
 * @param bus I2C bus state owned by the caller.
 * @param bus_handle Existing ESP-IDF I2C master bus handle.
 * @param dev_handle Existing ESP-IDF I2C device handle.
 * @return ESP_OK on success, otherwise ESP_ERR_INVALID_ARG.
 */
esp_err_t ICM42670_ESP_I2C_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_I2CBus *bus,
                                        i2c_master_bus_handle_t bus_handle,
                                        i2c_master_dev_handle_t dev_handle);

/**
 * @brief Release ESP-IDF I2C resources owned by this adapter.
 *
 * @param bus I2C bus state.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ICM42670_ESP_I2C_Deinit(ICM42670_ESP_I2CBus *bus);

/**
 * @brief Probe an I2C address on the adapter's bus.
 *
 * @param bus I2C bus state with a valid bus handle.
 * @param device_addr 7-bit I2C address to probe.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ICM42670_ESP_I2C_Probe(ICM42670_ESP_I2CBus *bus,
                                 uint8_t device_addr);

/**
 * @brief ESP-IDF I2C register-read callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_ESP_I2CBus.
 * @param reg_addr Register address to read from.
 * @param data Destination buffer.
 * @param len Number of bytes to read.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_ESP_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len);

/**
 * @brief ESP-IDF I2C register-write callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_ESP_I2CBus.
 * @param reg_addr Register address to write to.
 * @param data Source buffer.
 * @param len Number of bytes to write.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_ESP_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len);

/**
 * @brief Create an ESP-IDF SPI bus/device and populate ICM42670_Config.
 *
 * The adapter owns the created bus and device until ICM42670_ESP_SPI_Deinit()
 * is called.
 *
 * @param config Driver configuration to populate.
 * @param bus SPI bus state owned by the caller.
 * @param host_id ESP-IDF SPI host.
 * @param miso_pin MISO GPIO.
 * @param mosi_pin MOSI GPIO.
 * @param sclk_pin SCLK GPIO.
 * @param cs_pin Chip-select GPIO.
 * @param clock_speed_hz SPI clock speed, or 0 for the default.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ICM42670_ESP_SPI_Init(ICM42670_Config *config,
                                ICM42670_ESP_SPIBus *bus,
                                spi_host_device_t host_id, gpio_num_t miso_pin,
                                gpio_num_t mosi_pin, gpio_num_t sclk_pin,
                                gpio_num_t cs_pin, int clock_speed_hz);

/**
 * @brief Attach an existing ESP-IDF SPI device to ICM42670_Config.
 *
 * The adapter does not own attached handles and will not remove them during
 * deinit.
 *
 * @param config Driver configuration to populate.
 * @param bus SPI bus state owned by the caller.
 * @param host_id ESP-IDF SPI host used by dev_handle.
 * @param dev_handle Existing ESP-IDF SPI device handle.
 * @return ESP_OK on success, otherwise ESP_ERR_INVALID_ARG.
 */
esp_err_t ICM42670_ESP_SPI_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_SPIBus *bus,
                                        spi_host_device_t host_id,
                                        spi_device_handle_t dev_handle);

/**
 * @brief Release ESP-IDF SPI resources owned by this adapter.
 *
 * @param bus SPI bus state.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ICM42670_ESP_SPI_Deinit(ICM42670_ESP_SPIBus *bus);

/**
 * @brief ESP-IDF SPI register-read callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_ESP_SPIBus.
 * @param reg_addr Register address to read from.
 * @param data Destination buffer.
 * @param len Number of bytes to read.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_ESP_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len);

/**
 * @brief ESP-IDF SPI register-write callback for ICM42670_Config.
 *
 * @param handle Pointer to ICM42670_ESP_SPIBus.
 * @param reg_addr Register address to write to.
 * @param data Source buffer.
 * @param len Number of bytes to write.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
int8_t ICM42670_ESP_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_ESP_IDF_H */
