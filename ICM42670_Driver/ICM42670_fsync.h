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

typedef enum {
  ICM42670_FSYNC_RISING_EDGE = 0U,
  ICM42670_FSYNC_FALLING_EDGE = 1U,
} ICM42670_FsyncPolarity_t;

typedef enum {
  ICM42670_FSYNC_TAG_NONE = 0U,
  ICM42670_FSYNC_TAG_TEMP = 1U,
  ICM42670_FSYNC_TAG_GYRO_X = 2U,
  ICM42670_FSYNC_TAG_GYRO_Y = 3U,
  ICM42670_FSYNC_TAG_GYRO_Z = 4U,
  ICM42670_FSYNC_TAG_ACCEL_X = 5U,
  ICM42670_FSYNC_TAG_ACCEL_Y = 6U,
  ICM42670_FSYNC_TAG_ACCEL_Z = 7U,
} ICM42670_FsyncTag_t;

typedef enum {
  ICM42670_FSYNC_CLEAR_ON_SENSOR_UPDATE = 0U,
  ICM42670_FSYNC_CLEAR_ON_TAGGED_LSB_READ = 1U,
} ICM42670_FsyncClearMode_t;

typedef enum {
  ICM42670_FSYNC_INT_NONE = 0U,
  ICM42670_FSYNC_INT1 = 1U,
  ICM42670_FSYNC_INT2 = 2U,
} ICM42670_FsyncInterrupt_t;

typedef struct {
  ICM42670_FsyncPolarity_t polarity;
  ICM42670_FsyncTag_t tag;
  ICM42670_FsyncClearMode_t clear_mode;
  ICM42670_FsyncInterrupt_t interrupt_pin;
} ICM42670_FsyncConfig_t;

typedef struct {
  uint8_t event_detected;
  uint8_t raw_status;
  uint16_t timestamp_delta_us;
} ICM42670_FsyncData_t;

/**
 * @brief Enable FSYNC timestamp capture with simple defaults.
 *
 * Passing NULL for fsync_config selects rising-edge FSYNC, no UI data LSB tag,
 * clear-on-sensor-update behavior, and no interrupt routing.
 */
ICM42670_Status_t
ICM42670_Enable_Fsync(const ICM42670_Config *config,
                      const ICM42670_FsyncConfig_t *fsync_config);

ICM42670_Status_t ICM42670_Disable_Fsync(const ICM42670_Config *config);

/**
 * @brief Read the read-clear FSYNC event status and latest FSYNC timestamp.
 */
ICM42670_Status_t ICM42670_Read_Fsync(const ICM42670_Config *config,
                                      ICM42670_FsyncData_t *fsync_data);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_FSYNC_H */
