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

typedef struct {

  ICM42670_Odr_t accel_odr;    // Output Data Rate for accelerometer
  ICM42670_AccelFS_t accel_fs; // Full Scale range for accelerometer
  ICM42670_Odr_t gyro_odr;     // Output Data Rate for gyroscope
  ICM42670_GyroFS_t gyro_fs;   // Full Scale range for gyroscope

  void *handle; // User-defined handle for read/write/delay functions (e.g., I2C
                // or SPI handle / BUS type)
  int8_t (*read_reg)(void *handle, uint8_t reg_addr, const uint8_t *data,
                     uint16_t len);

  int8_t (*write_reg)(void *handle, uint8_t reg_addr, uint8_t *data,
                      uint16_t len);
  void (*delay_ms)(uint32_t ms);

} ICM42670_Config;

ICM42670_Status_t ICM42670_Init(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_ReadAccelRaw(const ICM42670_Config *config,
                                        int16_t accel_raw[6]);

ICM42670_Status_t ICM42670_ReadGyroRaw(const ICM42670_Config *config,
                                       int16_t gyro_raw[6]);

ICM42670_Status_t ICM42670_ReadTempRaw(const ICM42670_Config *config,
                                       int16_t *temp_raw);

ICM42670_Status_t ICM42670_ReadAccelG(const ICM42670_Config *config,
                                      ICM42670_Accel_t *accel);

ICM42670_Status_t ICM42670_ReadGyroDps(const ICM42670_Config *config,
                                       ICM42670_Gyro_t *gyro);

ICM42670_Status_t ICM42670_ReadTempC(const ICM42670_Config *config,
                                     float *temp_c);
#ifdef __cplusplus
}
#endif

#endif /* ICM42670_REGISTERMAP_H */
