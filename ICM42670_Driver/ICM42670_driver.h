/**
 * @file ICM42670_driver.h
 * @brief Core ICM-42670-P configuration and sensor read API.
 *
 * The core driver is platform independent. Applications provide register
 * read/write callbacks and a millisecond delay callback through
 * ICM42670_Config.
 */

#ifndef ICM42670_DRIVER_H
#define ICM42670_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ICM42670_registermap.h"
#include <stdint.h>

/**
 * @brief Return codes used by the ICM-42670-P driver.
 */
typedef enum {
  ICM42670_OK = 0,      /**< Operation completed successfully. */
  ICM42670_ERROR = -1,  /**< Invalid argument or bus/register error. */
  ICM42670_BUSY = -2,   /**< Device data is not ready yet. */
} ICM42670_Status_t;

/**
 * @brief Scaled accelerometer sample in g.
 */
typedef struct {
  float x_g; /**< X-axis acceleration in g. */
  float y_g; /**< Y-axis acceleration in g. */
  float z_g; /**< Z-axis acceleration in g. */
} ICM42670_Accel_t;

/**
 * @brief Scaled gyroscope sample in degrees per second.
 */
typedef struct {
  float x_dps; /**< X-axis angular rate in degrees per second. */
  float y_dps; /**< Y-axis angular rate in degrees per second. */
  float z_dps; /**< Z-axis angular rate in degrees per second. */
} ICM42670_Gyro_t;

/**
 * @brief Raw gyroscope bias offsets subtracted by ICM42670_ReadGyroDps().
 */
typedef struct {
  int16_t x_raw_offset; /**< X-axis raw-count offset. */
  int16_t y_raw_offset; /**< Y-axis raw-count offset. */
  int16_t z_raw_offset; /**< Z-axis raw-count offset. */
} ICM42670_Gyro_Offsets_t;


/**
 * @brief Accelerometer full-scale range values for ACCEL_CONFIG0.
 */
typedef enum {
  ICM42670_ACCEL_FS_16G = 0x00U,
  ICM42670_ACCEL_FS_8G = 0x20U,
  ICM42670_ACCEL_FS_4G = 0x40U,
  ICM42670_ACCEL_FS_2G = 0x60U,
} ICM42670_AccelFS_t;

/**
 * @brief Gyroscope full-scale range values for GYRO_CONFIG0.
 */
typedef enum {
  ICM42670_GYRO_FS_2000_DPS = 0x00U,
  ICM42670_GYRO_FS_1000_DPS = 0x20U,
  ICM42670_GYRO_FS_500_DPS = 0x40U,
  ICM42670_GYRO_FS_250_DPS = 0x60U
} ICM42670_GyroFS_t;

/**
 * @brief Output data rate values for accelerometer and gyroscope setup.
 */
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

/**
 * @brief UI low-pass filter bandwidth selections.
 */
typedef enum {
  ICM42670_LPF_BYPASSED = 0x00U,
  ICM42670_LPF_180_HZ = 0x01U,
  ICM42670_LPF_121_HZ = 0x02U,
  ICM42670_LPF_73_HZ = 0x03U,
  ICM42670_LPF_53_HZ = 0x04U,
  ICM42670_LPF_34_HZ = 0x05U,
  ICM42670_LPF_25_HZ = 0x06U,
  ICM42670_LPF_16_HZ = 0x07U,
} ICM42670_Lpf_t;

/**
 * @brief Standard PWR_MGMT0 power states exposed by the driver.
 */
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


/**
 * @brief Platform-independent driver configuration.
 *
 * Fill this structure directly or with a platform adapter helper before
 * calling ICM42670_Init().
 */
typedef struct {

  ICM42670_Odr_t accel_odr;    /**< Accelerometer output data rate. */
  ICM42670_AccelFS_t accel_fs; /**< Accelerometer full-scale range. */
  ICM42670_Odr_t gyro_odr;     /**< Gyroscope output data rate. */
  ICM42670_GyroFS_t gyro_fs;   /**< Gyroscope full-scale range. */

  ICM42670_Gyro_Offsets_t gyro_offsets; /**< Optional raw gyro offsets. */

  void *handle; /**< User context passed to read_reg and write_reg. */
  int8_t (*read_reg)(void *handle, uint8_t reg_addr, uint8_t *data,
                     uint16_t len); /**< Read len bytes starting at reg_addr. */

  int8_t (*write_reg)(void *handle, uint8_t reg_addr, const uint8_t *data,
                      uint16_t len); /**< Write len bytes starting at reg_addr. */
  void (*delay_ms)(uint32_t ms); /**< Blocking millisecond delay callback. */

} ICM42670_Config;

/**
 * @brief Initialize the device and enter 6-axis low-noise mode.
 *
 * The function validates callbacks, checks WHO_AM_I, normalizes invalid
 * range/ODR values to defaults, writes accel/gyro configuration, and enables
 * both sensors.
 *
 * @param config Driver configuration with valid callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Init(ICM42670_Config *config);

/**
 * @brief Switch the ICM-42670-P to one of the standard datasheet power modes.
 *
 * This writes PWR_MGMT0 and then waits 1 ms so the datasheet's 200 us
 * no-register-write window is respected when either sensor transitions from
 * off to an active mode. Gyro off/on timing and accel LP ODR/filter
 * restrictions remain caller responsibilities.
 *
 * @param config Driver configuration with write and delay callbacks.
 * @param state Target power state.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetPowerState(ICM42670_Config *config,
                                         ICM42670_PowerState_t state);

/**
 * @brief Set the accelerometer full-scale range.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param accel_fs Accelerometer range selection.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetAccelRange(ICM42670_Config *config,
                                         ICM42670_AccelFS_t accel_fs);

/**
 * @brief Set the gyroscope full-scale range.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param gyro_fs Gyroscope range selection.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetGyroRange(ICM42670_Config *config,
                                        ICM42670_GyroFS_t gyro_fs);

/**
 * @brief Set the accelerometer output data rate.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param odr Accelerometer output data rate.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetAccelOdr(ICM42670_Config *config,
                                       ICM42670_Odr_t odr);

/**
 * @brief Set the gyroscope output data rate.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param odr Gyroscope output data rate.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetGyroOdr(ICM42670_Config *config,
                                      ICM42670_Odr_t odr);

/**
 * @brief Set the accelerometer UI low-pass filter bandwidth.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param lpf Low-pass filter selection.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetAccelLpf(ICM42670_Config *config,
                                       ICM42670_Lpf_t lpf);

/**
 * @brief Set the gyroscope UI low-pass filter bandwidth.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param lpf Low-pass filter selection.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_SetGyroLpf(ICM42670_Config *config,
                                      ICM42670_Lpf_t lpf);

/**
 * @brief Read raw accelerometer counts.
 *
 * @param config Driver configuration with a read callback.
 * @param accel_raw Destination array: X, Y, Z raw counts.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadAccelRaw(const ICM42670_Config *config,
                                        int16_t accel_raw[3]);

/**
 * @brief Read raw gyroscope counts.
 *
 * @param config Driver configuration with a read callback.
 * @param gyro_raw Destination array: X, Y, Z raw counts.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadGyroRaw(const ICM42670_Config *config,
                                       int16_t gyro_raw[3]);

/**
 * @brief Read the raw temperature register value.
 *
 * @param config Driver configuration with a read callback.
 * @param temp_raw Destination for the signed raw temperature count.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadTempRaw(const ICM42670_Config *config,
                                       int16_t *temp_raw);

/**
 * @brief Read accelerometer data converted to g.
 *
 * @param config Driver configuration with current accel_fs value.
 * @param accel Destination for scaled acceleration.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadAccelG(const ICM42670_Config *config,
                                      ICM42670_Accel_t *accel);

/**
 * @brief Read gyroscope data converted to degrees per second.
 *
 * Configured gyro_offsets are subtracted before scaling.
 *
 * @param config Driver configuration with current gyro_fs value.
 * @param gyro Destination for scaled angular rate.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadGyroDps(const ICM42670_Config *config,
                                       ICM42670_Gyro_t *gyro);

/**
 * @brief Read temperature converted to degrees Celsius.
 *
 * @param config Driver configuration with a read callback.
 * @param temp_c Destination for temperature in degrees Celsius.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_ReadTempC(const ICM42670_Config *config,
                                     float *temp_c);

/**
 * @brief Estimate gyroscope bias while the board is stationary.
 *
 * This averages raw gyro samples and stores the offsets in config.
 *
 * @param config Driver configuration with read and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Gyro_Calibration(ICM42670_Config *config);
#ifdef __cplusplus
}
#endif

#endif /* ICM42670_DRIVER_H */
