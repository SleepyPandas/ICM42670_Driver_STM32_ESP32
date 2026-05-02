/**
 * @file ICM42670_fifo.c
 * @brief FIFO and interrupt routing functions for the ICM-42670-P.
 */

#include "ICM42670_fifo.h"
#include <stddef.h>

#define ICM42670_FIFO_MREG_DELAY_MS 1U
#define ICM42670_FIFO_FLUSH_DELAY_MS 1U

static int16_t CombineBytes(uint8_t msb, uint8_t lsb) {
  return (int16_t)(((uint16_t)msb << 8U) | (uint16_t)lsb);
}

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

static ICM42670_Status_t UpdateRegBits(const ICM42670_Config *config,
                                       uint8_t reg_addr, uint8_t mask,
                                       uint8_t field_value) {
  uint8_t value = 0U;

  if (ReadReg(config, reg_addr, &value, 1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  value = (uint8_t)((value & (uint8_t)~mask) | (field_value & mask));
  return WriteReg(config, reg_addr, value);
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

  config->delay_ms(ICM42670_FIFO_MREG_DELAY_MS);

  if (ReadReg(config, ICM42670_REG_M_R, value, 1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_FIFO_MREG_DELAY_MS);
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

  config->delay_ms(ICM42670_FIFO_MREG_DELAY_MS);
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

static ICM42670_Status_t ReadFifoCount(const ICM42670_Config *config,
                                       uint16_t *fifo_count) {
  uint8_t count_low = 0U;
  uint8_t count_high = 0U;

  if (fifo_count == NULL) {
    return ICM42670_ERROR;
  }

  if (ReadReg(config, ICM42670_REG_FIFO_COUNTL, &count_low, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ReadReg(config, ICM42670_REG_FIFO_COUNTH, &count_high, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *fifo_count = (uint16_t)(((uint16_t)count_high << 8U) | count_low);
  return ICM42670_OK;
}

static uint8_t FifoPacketSizeFromHeader(uint8_t header) {
  uint8_t has_accel = (header & ICM42670_FIFO_HEADER_ACCEL) != 0U;
  uint8_t has_gyro = (header & ICM42670_FIFO_HEADER_GYRO) != 0U;

  if ((header & ICM42670_FIFO_HEADER_MSG) != 0U) {
    return 1U;
  }

  if ((header & ICM42670_FIFO_HEADER_20) != 0U) {
    return 0U;
  }

  if ((has_accel != 0U) && (has_gyro != 0U)) {
    return ICM42670_FIFO_DEFAULT_PACKET_BYTES;
  }

  if ((has_accel != 0U) || (has_gyro != 0U)) {
    return 8U;
  }

  return 0U;
}

static void InitFifoPacket(ICM42670_FifoPacket_t *packet, uint8_t header) {
  packet->header = header;
  packet->has_accel = 0U;
  packet->has_gyro = 0U;
  packet->has_timestamp = 0U;
  packet->accel_raw[0] = 0;
  packet->accel_raw[1] = 0;
  packet->accel_raw[2] = 0;
  packet->gyro_raw[0] = 0;
  packet->gyro_raw[1] = 0;
  packet->gyro_raw[2] = 0;
  packet->temp_raw = 0;
  packet->timestamp = 0U;
}

static ICM42670_Status_t ParseFifoPacket(uint8_t header, const uint8_t *data,
                                         uint8_t data_len,
                                         ICM42670_FifoPacket_t *packet) {
  uint8_t offset = 0U;

  if ((data == NULL) || (packet == NULL)) {
    return ICM42670_ERROR;
  }

  InitFifoPacket(packet, header);

  if ((header & ICM42670_FIFO_HEADER_ACCEL) != 0U) {
    if ((uint8_t)(offset + ICM42670_ACCEL_DATA_LEN) > data_len) {
      return ICM42670_ERROR;
    }

    packet->has_accel = 1U;
    packet->accel_raw[0] = CombineBytes(data[offset], data[offset + 1U]);
    packet->accel_raw[1] = CombineBytes(data[offset + 2U], data[offset + 3U]);
    packet->accel_raw[2] = CombineBytes(data[offset + 4U], data[offset + 5U]);
    offset = (uint8_t)(offset + ICM42670_ACCEL_DATA_LEN);
  }

  if ((header & ICM42670_FIFO_HEADER_GYRO) != 0U) {
    if ((uint8_t)(offset + ICM42670_GYRO_DATA_LEN) > data_len) {
      return ICM42670_ERROR;
    }

    packet->has_gyro = 1U;
    packet->gyro_raw[0] = CombineBytes(data[offset], data[offset + 1U]);
    packet->gyro_raw[1] = CombineBytes(data[offset + 2U], data[offset + 3U]);
    packet->gyro_raw[2] = CombineBytes(data[offset + 4U], data[offset + 5U]);
    offset = (uint8_t)(offset + ICM42670_GYRO_DATA_LEN);
  }

  if (offset >= data_len) {
    return ICM42670_ERROR;
  }

  packet->temp_raw = (int8_t)data[offset];
  offset++;

  if ((packet->has_accel != 0U) && (packet->has_gyro != 0U) &&
      ((uint8_t)(offset + 2U) <= data_len)) {
    packet->has_timestamp = 1U;
    packet->timestamp = (uint16_t)(((uint16_t)data[offset] << 8U) |
                                   data[offset + 1U]);
  }

  return ICM42670_OK;
}

static ICM42670_Status_t SetIntPinDefaultIfUnset(
    const ICM42670_Config *config, ICM42670_IntPin_t pin) {
  uint8_t int_config = 0U;
  uint8_t pin_mask = 0U;
  uint8_t pin_default = 0U;

  if (ReadReg(config, ICM42670_REG_INT_CONFIG, &int_config, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  switch (pin) {
  case ICM42670_INT_PIN1:
    pin_mask = (uint8_t)(ICM42670_INT_CONFIG_INT1_LATCHED |
                         ICM42670_INT_CONFIG_INT1_PUSH_PULL |
                         ICM42670_INT_CONFIG_INT1_ACTIVE_HIGH);
    pin_default = (uint8_t)(ICM42670_INT_CONFIG_INT1_PUSH_PULL |
                            ICM42670_INT_CONFIG_INT1_ACTIVE_HIGH);
    break;
  case ICM42670_INT_PIN2:
    pin_mask = (uint8_t)(ICM42670_INT_CONFIG_INT2_LATCHED |
                         ICM42670_INT_CONFIG_INT2_PUSH_PULL |
                         ICM42670_INT_CONFIG_INT2_ACTIVE_HIGH);
    pin_default = (uint8_t)(ICM42670_INT_CONFIG_INT2_PUSH_PULL |
                            ICM42670_INT_CONFIG_INT2_ACTIVE_HIGH);
    break;
  default:
    return ICM42670_ERROR;
  }

  if ((int_config & pin_mask) != 0U) {
    return ICM42670_OK;
  }

  int_config = (uint8_t)(int_config | pin_default);
  return WriteReg(config, ICM42670_REG_INT_CONFIG, int_config);
}

ICM42670_Status_t ICM42670_FIFO_Init(const ICM42670_Config *config) {
  ICM42670_Status_t status;

  if (ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_FIFO_CONFIG5,
                           (uint8_t)(ICM42670_FIFO_CONFIG5_HIRES_EN |
                                     ICM42670_FIFO_CONFIG5_TMST_FSYNC_EN |
                                     ICM42670_FIFO_CONFIG5_GYRO_EN |
                                     ICM42670_FIFO_CONFIG5_ACCEL_EN |
                                     ICM42670_FIFO_CONFIG5_RESUME_PARTIAL_RD),
                           0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_FIFO_Flush(config);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_FIFO_SetWatermark(
      config, ICM42670_FIFO_DEFAULT_WATERMARK_BYTES);
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateRegBits(config, ICM42670_REG_INTF_CONFIG0,
                         ICM42670_INTF_CONFIG0_FIFO_COUNT_FORMAT_MASK,
                         ICM42670_INTF_CONFIG0_FIFO_COUNT_FORMAT_BYTES);
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateRegBits(config, ICM42670_REG_FIFO_CONFIG1,
                         (uint8_t)(ICM42670_FIFO_CONFIG1_MODE_MASK |
                                   ICM42670_FIFO_CONFIG1_BYPASS_MASK),
                         (uint8_t)(ICM42670_FIFO_CONFIG1_MODE_STREAM |
                                   ICM42670_FIFO_CONFIG1_BYPASS_DISABLE));
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_FIFO_CONFIG6,
                           (uint8_t)(ICM42670_FIFO_CONFIG6_EMPTY_INDICATOR_DIS |
                                     ICM42670_FIFO_CONFIG6_RCOSC_REQ_ON_FIFO_THS_DIS),
                           0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateMreg1Bits(config, ICM42670_MREG1_REG_FIFO_CONFIG5,
                           (uint8_t)(ICM42670_FIFO_CONFIG5_HIRES_EN |
                                     ICM42670_FIFO_CONFIG5_TMST_FSYNC_EN |
                                     ICM42670_FIFO_CONFIG5_GYRO_EN |
                                     ICM42670_FIFO_CONFIG5_ACCEL_EN |
                                     ICM42670_FIFO_CONFIG5_RESUME_PARTIAL_RD),
                           (uint8_t)(ICM42670_FIFO_CONFIG5_GYRO_EN |
                                     ICM42670_FIFO_CONFIG5_ACCEL_EN));
  if (status != ICM42670_OK) {
    return status;
  }

  return ICM42670_FIFO_Flush(config);
}

ICM42670_Status_t
ICM42670_FIFO_UseFullBuffer(const ICM42670_Config *config) {
  ICM42670_Status_t status;

  if (ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  status = UpdateRegBits(config, ICM42670_REG_APEX_CONFIG1,
                         ICM42670_APEX_CONFIG1_FEATURE_ENABLE_MASK, 0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = UpdateRegBits(config, ICM42670_REG_WOM_CONFIG,
                         ICM42670_WOM_CONFIG_WOM_EN, 0U);
  if (status != ICM42670_OK) {
    return status;
  }

  return UpdateMreg1Bits(config, ICM42670_MREG1_REG_SENSOR_CONFIG3,
                         ICM42670_SENSOR_CONFIG3_APEX_DISABLE,
                         ICM42670_SENSOR_CONFIG3_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_FIFO_SetWatermark(const ICM42670_Config *config,
                                             uint16_t watermark_bytes) {
  uint8_t watermark_low = 0U;
  uint8_t watermark_high = 0U;

  if (ValidateConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (watermark_bytes > ICM42670_FIFO_WATERMARK_MAX) {
    watermark_bytes = ICM42670_FIFO_WATERMARK_MAX;
  }

  watermark_low =
      (uint8_t)(watermark_bytes & ICM42670_FIFO_CONFIG2_WM_LOW_MASK);
  watermark_high = (uint8_t)((watermark_bytes >> 8U) &
                             ICM42670_FIFO_CONFIG3_WM_HIGH_MASK);

  if (WriteReg(config, ICM42670_REG_FIFO_CONFIG2, watermark_low) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return WriteReg(config, ICM42670_REG_FIFO_CONFIG3, watermark_high);
}

ICM42670_Status_t ICM42670_FIFO_Flush(const ICM42670_Config *config) {
  if ((ValidateConfig(config) != ICM42670_OK) || (config->delay_ms == NULL)) {
    return ICM42670_ERROR;
  }

  if (WriteReg(config, ICM42670_REG_SIGNAL_PATH_RESET,
               ICM42670_SIGNAL_PATH_FIFO_FLUSH) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  config->delay_ms(ICM42670_FIFO_FLUSH_DELAY_MS);
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_FIFO_Read(const ICM42670_Config *config,
                                     ICM42670_FifoPacket_t *packets,
                                     uint16_t max_packets,
                                     uint16_t *packets_read) {
  uint16_t fifo_count = 0U;
  uint16_t parsed_count = 0U;

  if ((ValidateConfig(config) != ICM42670_OK) || (packets_read == NULL) ||
      ((packets == NULL) && (max_packets > 0U))) {
    return ICM42670_ERROR;
  }

  *packets_read = 0U;

  if (max_packets == 0U) {
    return ICM42670_OK;
  }

  if (ReadFifoCount(config, &fifo_count) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  while ((parsed_count < max_packets) && (fifo_count > 0U)) {
    uint8_t header = 0U;
    uint8_t packet_size = 0U;
    uint8_t payload_len = 0U;
    uint8_t payload[ICM42670_FIFO_DEFAULT_PACKET_BYTES - 1U] = {0U};

    if (ReadReg(config, ICM42670_REG_FIFO_DATA, &header, 1U) !=
        ICM42670_OK) {
      return ICM42670_ERROR;
    }

    packet_size = FifoPacketSizeFromHeader(header);
    if (packet_size == 0U) {
      return ICM42670_ERROR;
    }

    if ((header & ICM42670_FIFO_HEADER_MSG) != 0U) {
      break;
    }

    if (packet_size > fifo_count) {
      break;
    }

    payload_len = (uint8_t)(packet_size - 1U);
    if (ReadReg(config, ICM42670_REG_FIFO_DATA, payload, payload_len) !=
        ICM42670_OK) {
      return ICM42670_ERROR;
    }

    if (ParseFifoPacket(header, payload, payload_len,
                        &packets[parsed_count]) != ICM42670_OK) {
      return ICM42670_ERROR;
    }

    fifo_count = (uint16_t)(fifo_count - packet_size);
    parsed_count++;
  }

  *packets_read = parsed_count;
  return ICM42670_OK;
}

ICM42670_Status_t
ICM42670_FIFO_RouteInterrupt(const ICM42670_Config *config,
                             ICM42670_IntPin_t pin,
                             ICM42670_FifoIntSource_t sources) {
  uint8_t reg_addr = 0U;
  uint8_t source_bits = (uint8_t)sources;
  const uint8_t fifo_int_mask =
      (uint8_t)(ICM42670_FIFO_INT_THRESHOLD | ICM42670_FIFO_INT_FULL);

  if (ValidateConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if ((source_bits & (uint8_t)~fifo_int_mask) != 0U) {
    return ICM42670_ERROR;
  }

  switch (pin) {
  case ICM42670_INT_PIN1:
    reg_addr = ICM42670_REG_INT_SOURCE0;
    break;
  case ICM42670_INT_PIN2:
    reg_addr = ICM42670_REG_INT_SOURCE3;
    break;
  default:
    return ICM42670_ERROR;
  }

  if (SetIntPinDefaultIfUnset(config, pin) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  return UpdateRegBits(config, reg_addr, fifo_int_mask, source_bits);
}
