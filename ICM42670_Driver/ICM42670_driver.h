/**
 * @file ICM42670_driver.h
 * @brief Basic ICM-42670-P driver functions and definitions.
 *
 *
 * This file defines the main functions for initializing and reading data from
 * the ICM-42670-P IMU chip.
 *
 */

#ifndef ICM42670_DRIVER_H
#define ICM42670_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

// Status codes for ICM42670 driver functions.

#include "ICM42670_registermap.h"
#include <stdint.h>

typedef enum {
  ICM42670_OK = 0,
  ICM42670_ERROR = -1,
  ICM42670_BUSY = -2,
} ICM42670_Status_t;

typedef struct {
  float x_g;
  float y_g;
  float z_g;
} ICM42670_Accel_t;

typedef struct {
  float x_dps;
  float y_dps;
  float z_dps;
} ICM42670_Gyro_t;

typedef struct {
  int16_t x_raw_offset;
  int16_t y_raw_offset;
  int16_t z_raw_offset;
} ICM42670_Gyro_Offsets_t;


typedef enum {
  ICM42670_ACCEL_FS_16G = 0x00U,
  ICM42670_ACCEL_FS_8G = 0x20U,
  ICM42670_ACCEL_FS_4G = 0x40U,
  ICM42670_ACCEL_FS_2G = 0x60U,
} ICM42670_AccelFS_t;

typedef enum {
  ICM42670_GYRO_FS_2000_DPS = 0x00U,
  ICM42670_GYRO_FS_1000_DPS = 0x20U,
  ICM42670_GYRO_FS_500_DPS = 0x40U,
  ICM42670_GYRO_FS_250_DPS = 0x60U
} ICM42670_GyroFS_t;

typedef enum {
  ICM42670_ODR_1600_HZ = 0x05U,
  ICM42670_ODR_800_HZ = 0x06U,
  ICM42670_ODR_400_HZ = 0x07U,
  ICM42670_ODR_200_HZ = 0x08U,
  ICM42670_ODR_100_HZ = 0x09U,
  ICM42670_ODR_50_HZ = 0x0AU,
  ICM42670_ODR_25_HZ = 0x0BU,
  ICM42670_ODR_12_5_HZ = 0x0CU,
  ICM42670_ACCEL_ODR_6_25_HZ = 0x0DU,
  ICM42670_ACCEL_ODR_3_125_HZ = 0x0EU,
  ICM42670_ACCEL_ODR_1_5625_HZ = 0x0FU,
} ICM42670_Odr_t;

typedef enum {
  /**
   * @brief Sleep mode: gyro off, accel off.
   *
   * Lowest normal sensor-off state. MREG access is not available in sleep
   * unless the RC oscillator is kept on with PWR_MGMT0.IDLE.
   */
  ICM42670_POWER_SLEEP = 0x00U,

  /**
   * @brief Standby mode: gyro drive on, accel off.
   *
   * Use when preparing gyro operation without accelerometer data. The gyro
   * should stay enabled for at least 45 ms once powered on, and callers should
   * wait more than 20 ms after powering it off before enabling it again.
   */
  ICM42670_POWER_STANDBY,

  /**
   * @brief Accelerometer low-power mode: gyro off, accel duty-cycled.
   *
   * Use for low-current, always-on motion sensing. Configure a valid
   * accelerometer ODR and LP averaging before entering this mode: 1600 Hz and
   * 800 Hz are LN-only, while 6.25 Hz, 3.125 Hz, and 1.5625 Hz are LP-only.
   * MREG access is not supported with the WUOSC LP clock unless IDLE enables
   * the RC oscillator.
   */
  ICM42670_POWER_ACCEL_LP,

  /**
   * @brief Accelerometer low-noise mode: gyro off, accel on.
   *
   * Use for accelerometer-only reads when lower noise is more important than
   * minimum current draw.
   */
  ICM42670_POWER_ACCEL_LN,

  /**
   * @brief Gyroscope low-noise mode: gyro on, accel off.
   *
   * Use for gyroscope-only reads. The ICM-42670-P does not provide a gyro
   * low-power measurement mode.
   */
  ICM42670_POWER_GYRO_LN,

  /**
   * @brief 6-axis low-noise mode: gyro on, accel on.
   *
   * Use when both accelerometer and gyroscope data are needed. This is the
   * default mode selected by ICM42670_Init().
   */
  ICM42670_POWER_6AXIS_LN,
} ICM42670_PowerState_t;




typedef struct {

  ICM42670_Odr_t accel_odr;    // Output Data Rate for accelerometer
  ICM42670_AccelFS_t accel_fs; // Full Scale range for accelerometer
  ICM42670_Odr_t gyro_odr;     // Output Data Rate for gyroscope
  ICM42670_GyroFS_t gyro_fs;   // Full Scale range for gyroscope

  ICM42670_Gyro_Offsets_t gyro_offsets; // Optional gyro offset values to apply to raw readings

  void *handle; // User-defined handle for read/write/delay functions (e.g., I2C
                // or SPI handle / BUS type)
  int8_t (*read_reg)(void *handle, uint8_t reg_addr, uint8_t *data,
                     uint16_t len);

  int8_t (*write_reg)(void *handle, uint8_t reg_addr, const uint8_t *data,
                      uint16_t len);
  void (*delay_ms)(uint32_t ms);

} ICM42670_Config;

ICM42670_Status_t ICM42670_Init(ICM42670_Config *config);

/**
 * @brief Switch the ICM-42670-P to one of the standard datasheet power modes.
 *
 * This writes PWR_MGMT0 and then waits 1 ms so the datasheet's 200 us
 * no-register-write window is respected when either sensor transitions from
 * off to an active mode. Gyro off/on timing and accel LP ODR/filter
 * restrictions remain caller responsibilities.
 */
ICM42670_Status_t ICM42670_SetPowerState(ICM42670_Config *config,
                                         ICM42670_PowerState_t state);

ICM42670_Status_t ICM42670_ReadAccelRaw(const ICM42670_Config *config,
                                        int16_t accel_raw[3]);

ICM42670_Status_t ICM42670_ReadGyroRaw(const ICM42670_Config *config,
                                       int16_t gyro_raw[3]);

ICM42670_Status_t ICM42670_ReadTempRaw(const ICM42670_Config *config,
                                       int16_t *temp_raw);

ICM42670_Status_t ICM42670_ReadAccelG(const ICM42670_Config *config,
                                      ICM42670_Accel_t *accel);

ICM42670_Status_t ICM42670_ReadGyroDps(const ICM42670_Config *config,
                                       ICM42670_Gyro_t *gyro);

ICM42670_Status_t ICM42670_ReadTempC(const ICM42670_Config *config,
                                     float *temp_c);

ICM42670_Status_t ICM42670_Gyro_Calibration(ICM42670_Config *config);
#ifdef __cplusplus
}
#endif

#endif /* ICM42670_REGISTERMAP_H */
