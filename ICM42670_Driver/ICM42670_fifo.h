/**
 * @file ICM42670_fifo.h
 * @brief Simple FIFO and interrupt routing API for the ICM-42670-P.
 */

#ifndef ICM42670_FIFO_H
#define ICM42670_FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include <stdint.h>

/**
 * @brief Default FIFO watermark used by ICM42670_FIFO_Init().
 */
#define ICM42670_FIFO_DEFAULT_WATERMARK_BYTES 64U

/**
 * @brief Maximum bytes in the default 16-bit accel + gyro FIFO packet.
 */
#define ICM42670_FIFO_DEFAULT_PACKET_BYTES 16U

/**
 * @brief ICM-42670-P interrupt output pins.
 */
typedef enum {
  ICM42670_INT_PIN1 = 0U, /**< INT1 output pin. */
  ICM42670_INT_PIN2 = 1U, /**< INT2 output pin. */
} ICM42670_IntPin_t;

/**
 * @brief FIFO interrupt sources that can be routed to INT1 or INT2.
 */
typedef enum {
  ICM42670_FIFO_INT_NONE = 0x00U, /**< Clear FIFO interrupt routing. */
  ICM42670_FIFO_INT_THRESHOLD = 0x04U, /**< FIFO watermark interrupt. */
  ICM42670_FIFO_INT_FULL = 0x02U, /**< FIFO full interrupt. */
  ICM42670_FIFO_INT_THRESHOLD_AND_FULL =
      (ICM42670_FIFO_INT_THRESHOLD | ICM42670_FIFO_INT_FULL), /**< Both FIFO sources. */
} ICM42670_FifoIntSource_t;

/**
 * @brief Parsed FIFO packet using the default 16-bit FIFO format.
 */
typedef struct {
  uint8_t header; /**< Raw FIFO packet header byte. */
  uint8_t has_accel; /**< Nonzero when accel_raw contains valid data. */
  uint8_t has_gyro; /**< Nonzero when gyro_raw contains valid data. */
  uint8_t has_timestamp; /**< Nonzero when timestamp contains valid data. */
  int16_t accel_raw[3]; /**< Raw X, Y, Z accelerometer counts. */
  int16_t gyro_raw[3]; /**< Raw X, Y, Z gyroscope counts. */
  int8_t temp_raw; /**< Raw FIFO temperature byte. */
  uint16_t timestamp; /**< Raw FIFO timestamp value. */
} ICM42670_FifoPacket_t;

/**
 * @brief Initialize the FIFO with simple 16-bit accel and gyro packet defaults.
 *
 * Configures stream mode, disables FIFO bypass, enables accel and gyro FIFO
 * packets, uses byte-based FIFO counts, applies a small watermark, and flushes
 * stale FIFO data before use.
 *
 * @param config Driver configuration with read, write, and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_FIFO_Init(const ICM42670_Config *config);

/**
 * @brief Disable APEX features so the FIFO can use the full 2.25 KB buffer.
 *
 * APEX and full FIFO capacity share SRAM. Call this only when APEX features are
 * not needed.
 *
 * @param config Driver configuration with read, write, and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_FIFO_UseFullBuffer(const ICM42670_Config *config);

/**
 * @brief Set the FIFO watermark level in bytes.
 *
 * The watermark is used by the FIFO threshold interrupt source. Values larger
 * than the 12-bit FIFO watermark field are clamped.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param watermark_bytes FIFO threshold in bytes.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_FIFO_SetWatermark(const ICM42670_Config *config,
                                             uint16_t watermark_bytes);

/**
 * @brief Flush all unread FIFO data.
 *
 * @param config Driver configuration with write and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_FIFO_Flush(const ICM42670_Config *config);

/**
 * @brief Read and parse available FIFO packets.
 *
 * Reads up to max_packets parsed FIFO records into packets and writes the
 * number of parsed packets to packets_read.
 *
 * @param config Driver configuration with a read callback.
 * @param packets Destination array for parsed packets.
 * @param max_packets Maximum number of packets to parse into packets.
 * @param packets_read Number of packets parsed by this call.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_FIFO_Read(const ICM42670_Config *config,
                                     ICM42670_FifoPacket_t *packets,
                                     uint16_t max_packets,
                                     uint16_t *packets_read);

/**
 * @brief Route FIFO threshold and/or FIFO full interrupts to INT1 or INT2.
 *
 * Passing ICM42670_FIFO_INT_NONE clears FIFO routing for the selected pin while
 * preserving other interrupt sources.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param pin Target interrupt pin.
 * @param sources FIFO interrupt sources to route.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t
ICM42670_FIFO_RouteInterrupt(const ICM42670_Config *config,
                             ICM42670_IntPin_t pin,
                             ICM42670_FifoIntSource_t sources);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_FIFO_H */
