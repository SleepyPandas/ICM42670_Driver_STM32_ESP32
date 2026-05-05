/**
 * @file ICM42670_fsync.h
 * @brief Simple FSYNC API for the ICM-42670-P.
 */

#ifndef ICM42670_FSYNC_H
#define ICM42670_FSYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_driver.h"
#include <stdint.h>

/**
 * @brief FSYNC input edge selection.
 */
typedef enum {
  ICM42670_FSYNC_RISING_EDGE = 0U, /**< Capture rising-edge FSYNC events. */
  ICM42670_FSYNC_FALLING_EDGE = 1U, /**< Capture falling-edge FSYNC events. */
} ICM42670_FsyncPolarity_t;

/**
 * @brief Optional UI data LSB tag for FSYNC events.
 */
typedef enum {
  ICM42670_FSYNC_TAG_NONE = 0U, /**< Do not tag sensor data LSBs. */
  ICM42670_FSYNC_TAG_TEMP = 1U, /**< Tag temperature LSB. */
  ICM42670_FSYNC_TAG_GYRO_X = 2U, /**< Tag gyro X LSB. */
  ICM42670_FSYNC_TAG_GYRO_Y = 3U, /**< Tag gyro Y LSB. */
  ICM42670_FSYNC_TAG_GYRO_Z = 4U, /**< Tag gyro Z LSB. */
  ICM42670_FSYNC_TAG_ACCEL_X = 5U, /**< Tag accel X LSB. */
  ICM42670_FSYNC_TAG_ACCEL_Y = 6U, /**< Tag accel Y LSB. */
  ICM42670_FSYNC_TAG_ACCEL_Z = 7U, /**< Tag accel Z LSB. */
} ICM42670_FsyncTag_t;

/**
 * @brief FSYNC event flag clear behavior.
 */
typedef enum {
  ICM42670_FSYNC_CLEAR_ON_SENSOR_UPDATE = 0U, /**< Clear at next sensor update. */
  ICM42670_FSYNC_CLEAR_ON_TAGGED_LSB_READ = 1U, /**< Clear when tagged LSB is read. */
} ICM42670_FsyncClearMode_t;

/**
 * @brief FSYNC interrupt routing target.
 */
typedef enum {
  ICM42670_FSYNC_INT_NONE = 0U, /**< Do not route FSYNC to an interrupt pin. */
  ICM42670_FSYNC_INT1 = 1U, /**< Route FSYNC to INT1. */
  ICM42670_FSYNC_INT2 = 2U, /**< Route FSYNC to INT2. */
} ICM42670_FsyncInterrupt_t;

/**
 * @brief FSYNC configuration passed to ICM42670_Enable_Fsync().
 */
typedef struct {
  ICM42670_FsyncPolarity_t polarity; /**< Input edge to capture. */
  ICM42670_FsyncTag_t tag; /**< Optional sensor data LSB tag target. */
  ICM42670_FsyncClearMode_t clear_mode; /**< FSYNC flag clear behavior. */
  ICM42670_FsyncInterrupt_t interrupt_pin; /**< Optional interrupt routing. */
} ICM42670_FsyncConfig_t;

/**
 * @brief FSYNC event status and timestamp data.
 */
typedef struct {
  uint8_t event_detected; /**< Nonzero when the last status read saw FSYNC. */
  uint8_t raw_status; /**< Raw INT_STATUS value read from the device. */
  uint16_t timestamp_delta_us; /**< FSYNC timestamp delta in microseconds. */
} ICM42670_FsyncData_t;

/**
 * @brief Enable FSYNC timestamp capture with simple defaults.
 *
 * Passing NULL for fsync_config selects rising-edge FSYNC, no UI data LSB tag,
 * clear-on-sensor-update behavior, and no interrupt routing.
 *
 * @param config Driver configuration with read, write, and delay callbacks.
 * @param fsync_config Optional FSYNC configuration, or NULL for defaults.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t
ICM42670_Enable_Fsync(const ICM42670_Config *config,
                      const ICM42670_FsyncConfig_t *fsync_config);

/**
 * @brief Disable FSYNC timestamp capture and clear FSYNC interrupt routing.
 *
 * @param config Driver configuration with read, write, and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Disable_Fsync(const ICM42670_Config *config);

/**
 * @brief Read the read-clear FSYNC event status and latest FSYNC timestamp.
 *
 * @param config Driver configuration with a read callback.
 * @param fsync_data Destination for event status and timestamp data.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Fsync(const ICM42670_Config *config,
                                      ICM42670_FsyncData_t *fsync_data);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_FSYNC_H */
