/**
 * @file ICM42670_esp_idf.c
 * @brief ESP-IDF I2C and SPI transport adapter for the ICM-42670-P driver.
 */

#include "ICM42670_esp_idf.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define ICM42670_ESP_SPI_READ_BIT 0x80U
#define ICM42670_ESP_MAX_TRANSFER_LEN 32U

static void ICM42670_ESP_DelayMs(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
}

static uint32_t ICM42670_ESP_TimeoutMs(uint32_t timeout_ms) {
  return (timeout_ms == 0U) ? 1000U : timeout_ms;
}

static void ICM42670_ESP_ApplyConfig(ICM42670_Config *config, void *handle,
                                     int8_t (*read_reg)(void *, uint8_t,
                                                        uint8_t *, uint16_t),
                                     int8_t (*write_reg)(void *, uint8_t,
                                                         const uint8_t *,
                                                         uint16_t)) {
  config->handle = handle;
  config->read_reg = read_reg;
  config->write_reg = write_reg;
  config->delay_ms = ICM42670_ESP_DelayMs;
}

esp_err_t ICM42670_ESP_I2C_Init(ICM42670_Config *config,
                                ICM42670_ESP_I2CBus *bus, int i2c_port,
                                gpio_num_t sda_pin, gpio_num_t scl_pin,
                                uint8_t device_addr,
                                uint32_t scl_speed_hz) {
  esp_err_t err;
  i2c_master_bus_config_t bus_config = {0};
  i2c_device_config_t device_config = {0};

  if ((config == NULL) || (bus == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(bus, 0, sizeof(*bus));

  if (scl_speed_hz == 0U) {
    scl_speed_hz = ICM42670_ESP_DEFAULT_I2C_SPEED_HZ;
  }

  bus_config.i2c_port = i2c_port;
  bus_config.sda_io_num = sda_pin;
  bus_config.scl_io_num = scl_pin;
  bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
  bus_config.glitch_ignore_cnt = 7U;
  bus_config.flags.enable_internal_pullup = true;

  err = i2c_new_master_bus(&bus_config, &bus->bus_handle);
  if (err != ESP_OK) {
    return err;
  }
  bus->owns_bus = true;

  device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  device_config.device_address = device_addr;
  device_config.scl_speed_hz = scl_speed_hz;

  err = i2c_master_bus_add_device(bus->bus_handle, &device_config,
                                  &bus->dev_handle);
  if (err != ESP_OK) {
    (void)ICM42670_ESP_I2C_Deinit(bus);
    return err;
  }
  bus->owns_device = true;
  bus->timeout_ms = 1000U;

  ICM42670_ESP_ApplyConfig(config, bus, ICM42670_ESP_I2C_ReadReg,
                           ICM42670_ESP_I2C_WriteReg);

  return ESP_OK;
}

esp_err_t ICM42670_ESP_I2C_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_I2CBus *bus,
                                        i2c_master_bus_handle_t bus_handle,
                                        i2c_master_dev_handle_t dev_handle) {
  if ((config == NULL) || (bus == NULL) || (dev_handle == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(bus, 0, sizeof(*bus));
  bus->bus_handle = bus_handle;
  bus->dev_handle = dev_handle;
  bus->timeout_ms = 1000U;

  ICM42670_ESP_ApplyConfig(config, bus, ICM42670_ESP_I2C_ReadReg,
                           ICM42670_ESP_I2C_WriteReg);

  return ESP_OK;
}

esp_err_t ICM42670_ESP_I2C_Deinit(ICM42670_ESP_I2CBus *bus) {
  esp_err_t first_err = ESP_OK;

  if (bus == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  if (bus->owns_device && (bus->dev_handle != NULL)) {
    first_err = i2c_master_bus_rm_device(bus->dev_handle);
  }

  if (bus->owns_bus && (bus->bus_handle != NULL)) {
    esp_err_t err = i2c_del_master_bus(bus->bus_handle);
    if (first_err == ESP_OK) {
      first_err = err;
    }
  }

  memset(bus, 0, sizeof(*bus));
  return first_err;
}

esp_err_t ICM42670_ESP_I2C_Probe(ICM42670_ESP_I2CBus *bus,
                                 uint8_t device_addr) {
  if ((bus == NULL) || (bus->bus_handle == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  return i2c_master_probe(bus->bus_handle, device_addr,
                          (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
}

int8_t ICM42670_ESP_I2C_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len) {
  ICM42670_ESP_I2CBus *bus = (ICM42670_ESP_I2CBus *)handle;
  esp_err_t err;

  if ((bus == NULL) || (bus->dev_handle == NULL) || (data == NULL) ||
      (len == 0U)) {
    return ICM42670_ERROR;
  }

  err = i2c_master_transmit_receive(
      bus->dev_handle, &reg_addr, 1U, data, len,
      (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
  if (err == ESP_ERR_TIMEOUT && bus->bus_handle != NULL) {
    (void)i2c_master_bus_reset(bus->bus_handle);
    err = i2c_master_transmit_receive(
        bus->dev_handle, &reg_addr, 1U, data, len,
        (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
  } else if (err != ESP_OK) {
    err = i2c_master_transmit_receive(
        bus->dev_handle, &reg_addr, 1U, data, len,
        (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
  }

  return (err == ESP_OK) ? ICM42670_OK : ICM42670_ERROR;
}

int8_t ICM42670_ESP_I2C_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len) {
  ICM42670_ESP_I2CBus *bus = (ICM42670_ESP_I2CBus *)handle;
  uint8_t tx[1U + ICM42670_ESP_MAX_TRANSFER_LEN] = {0};
  esp_err_t err;

  if ((bus == NULL) || (bus->dev_handle == NULL) || (data == NULL) ||
      (len == 0U) || (len > ICM42670_ESP_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr;
  memcpy(&tx[1], data, len);

  err = i2c_master_transmit(bus->dev_handle, tx, (size_t)(len + 1U),
                            (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
  if (err == ESP_ERR_TIMEOUT && bus->bus_handle != NULL) {
    (void)i2c_master_bus_reset(bus->bus_handle);
    err = i2c_master_transmit(bus->dev_handle, tx, (size_t)(len + 1U),
                              (int)ICM42670_ESP_TimeoutMs(bus->timeout_ms));
  }

  return (err == ESP_OK) ? ICM42670_OK : ICM42670_ERROR;
}

esp_err_t ICM42670_ESP_SPI_Init(ICM42670_Config *config,
                                ICM42670_ESP_SPIBus *bus,
                                spi_host_device_t host_id, gpio_num_t miso_pin,
                                gpio_num_t mosi_pin, gpio_num_t sclk_pin,
                                gpio_num_t cs_pin, int clock_speed_hz) {
  esp_err_t err;
  spi_bus_config_t bus_config = {0};
  spi_device_interface_config_t device_config = {0};

  if ((config == NULL) || (bus == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(bus, 0, sizeof(*bus));

  if (clock_speed_hz <= 0) {
    clock_speed_hz = ICM42670_ESP_DEFAULT_SPI_SPEED_HZ;
  }

  bus_config.miso_io_num = miso_pin;
  bus_config.mosi_io_num = mosi_pin;
  bus_config.sclk_io_num = sclk_pin;
  bus_config.quadwp_io_num = -1;
  bus_config.quadhd_io_num = -1;
  bus_config.max_transfer_sz = (int)(1U + ICM42670_ESP_MAX_TRANSFER_LEN);

  err = spi_bus_initialize(host_id, &bus_config, SPI_DMA_DISABLED);
  if (err != ESP_OK) {
    return err;
  }
  bus->host_id = host_id;
  bus->owns_bus = true;

  device_config.clock_speed_hz = clock_speed_hz;
  device_config.mode = 0;
  device_config.spics_io_num = cs_pin;
  device_config.queue_size = 1;

  err = spi_bus_add_device(host_id, &device_config, &bus->dev_handle);
  if (err != ESP_OK) {
    (void)ICM42670_ESP_SPI_Deinit(bus);
    return err;
  }
  bus->owns_device = true;
  bus->timeout_ms = 1000U;

  ICM42670_ESP_ApplyConfig(config, bus, ICM42670_ESP_SPI_ReadReg,
                           ICM42670_ESP_SPI_WriteReg);

  return ESP_OK;
}

esp_err_t ICM42670_ESP_SPI_AttachDevice(ICM42670_Config *config,
                                        ICM42670_ESP_SPIBus *bus,
                                        spi_host_device_t host_id,
                                        spi_device_handle_t dev_handle) {
  if ((config == NULL) || (bus == NULL) || (dev_handle == NULL)) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(bus, 0, sizeof(*bus));
  bus->host_id = host_id;
  bus->dev_handle = dev_handle;
  bus->timeout_ms = 1000U;

  ICM42670_ESP_ApplyConfig(config, bus, ICM42670_ESP_SPI_ReadReg,
                           ICM42670_ESP_SPI_WriteReg);

  return ESP_OK;
}

esp_err_t ICM42670_ESP_SPI_Deinit(ICM42670_ESP_SPIBus *bus) {
  esp_err_t first_err = ESP_OK;

  if (bus == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  if (bus->owns_device && (bus->dev_handle != NULL)) {
    first_err = spi_bus_remove_device(bus->dev_handle);
  }

  if (bus->owns_bus) {
    esp_err_t err = spi_bus_free(bus->host_id);
    if (first_err == ESP_OK) {
      first_err = err;
    }
  }

  memset(bus, 0, sizeof(*bus));
  return first_err;
}

int8_t ICM42670_ESP_SPI_ReadReg(void *handle, uint8_t reg_addr, uint8_t *data,
                                uint16_t len) {
  ICM42670_ESP_SPIBus *bus = (ICM42670_ESP_SPIBus *)handle;
  uint8_t tx[1U + ICM42670_ESP_MAX_TRANSFER_LEN] = {0};
  uint8_t rx[1U + ICM42670_ESP_MAX_TRANSFER_LEN] = {0};
  spi_transaction_t transaction = {0};

  if ((bus == NULL) || (bus->dev_handle == NULL) || (data == NULL) ||
      (len == 0U) || (len > ICM42670_ESP_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr | ICM42670_ESP_SPI_READ_BIT;
  transaction.length = (size_t)(len + 1U) * 8U;
  transaction.tx_buffer = tx;
  transaction.rx_buffer = rx;

  if (spi_device_transmit(bus->dev_handle, &transaction) != ESP_OK) {
    return ICM42670_ERROR;
  }

  memcpy(data, &rx[1], len);
  return ICM42670_OK;
}

int8_t ICM42670_ESP_SPI_WriteReg(void *handle, uint8_t reg_addr,
                                 const uint8_t *data, uint16_t len) {
  ICM42670_ESP_SPIBus *bus = (ICM42670_ESP_SPIBus *)handle;
  uint8_t tx[1U + ICM42670_ESP_MAX_TRANSFER_LEN] = {0};
  spi_transaction_t transaction = {0};

  if ((bus == NULL) || (bus->dev_handle == NULL) || (data == NULL) ||
      (len == 0U) || (len > ICM42670_ESP_MAX_TRANSFER_LEN)) {
    return ICM42670_ERROR;
  }

  tx[0] = reg_addr & (uint8_t)~ICM42670_ESP_SPI_READ_BIT;
  memcpy(&tx[1], data, len);

  transaction.length = (size_t)(len + 1U) * 8U;
  transaction.tx_buffer = tx;

  return (spi_device_transmit(bus->dev_handle, &transaction) == ESP_OK)
             ? ICM42670_OK
             : ICM42670_ERROR;
}
