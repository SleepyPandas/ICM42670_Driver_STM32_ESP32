/**
 * @file ICM42670_apex.h
 * @brief Optional APEX motion feature API for the ICM-42670-P.
 *
 * APEX features use the device's internal DMP. Call ICM42670_Init_Apex()
 * before enabling pedometer, tilt, low-g, freefall, wake-on-motion, or
 * significant-motion features.
 */

#ifndef ICM42670_APEX_H
#define ICM42670_APEX_H

#include "ICM42670_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pedometer activity classification.
 */
typedef enum {
  ICM42670_APEX_ACTIVITY_UNKNOWN = 0U,
  ICM42670_APEX_ACTIVITY_WALK = 1U,
  ICM42670_APEX_ACTIVITY_RUN = 2U,
} ICM42670_ApexActivity_t;

/**
 * @brief Pedometer configuration.
 */
typedef struct {
  uint8_t slow_walk_enable; /**< Nonzero enables slow-walk detection tuning. */
} ICM42670_PedoConfig_t;

/**
 * @brief Pedometer output data.
 */
typedef struct {
  uint16_t step_count; /**< Total step count reported by APEX. */
  uint8_t cadence_raw; /**< Raw cadence byte from APEX data registers. */
  ICM42670_ApexActivity_t activity; /**< Current walk/run classification. */
  uint8_t step_detected; /**< Nonzero when a step event was reported. */
  uint8_t overflow; /**< Nonzero when the step counter overflowed. */
} ICM42670_PedoData_t;

/**
 * @brief Tilt detection configuration.
 */
typedef struct {
  uint8_t wait_time_s; /**< Requested tilt wait time in seconds. */
} ICM42670_TiltConfig_t;

/**
 * @brief Low-g detection configuration.
 */
typedef struct {
  uint16_t threshold_mg; /**< Requested low-g threshold in mg. */
  uint8_t sample_count; /**< Number of samples below threshold. */
} ICM42670_LowGConfig_t;

/**
 * @brief Freefall detection configuration.
 */
typedef struct {
  uint16_t min_distance_cm; /**< Minimum fall distance in centimeters. */
  uint16_t max_distance_cm; /**< Maximum fall distance in centimeters. */
  uint16_t debounce_ms; /**< Freefall debounce time in milliseconds. */
} ICM42670_FreeFallConfig_t;

/**
 * @brief Freefall output data.
 */
typedef struct {
  uint8_t detected; /**< Nonzero when a freefall event was reported. */
  uint16_t duration_samples; /**< Raw freefall duration in APEX samples. */
} ICM42670_FreeFallData_t;

/**
 * @brief Wake-on-motion threshold configuration.
 */
typedef struct {
  uint16_t x_threshold_mg; /**< X-axis motion threshold in mg. */
  uint16_t y_threshold_mg; /**< Y-axis motion threshold in mg. */
  uint16_t z_threshold_mg; /**< Z-axis motion threshold in mg. */
} ICM42670_WakeOnMotionConfig_t;

/**
 * @brief Wake-on-motion output flags.
 */
typedef struct {
  uint8_t x_detected; /**< Nonzero when X-axis motion was reported. */
  uint8_t y_detected; /**< Nonzero when Y-axis motion was reported. */
  uint8_t z_detected; /**< Nonzero when Z-axis motion was reported. */
} ICM42670_WakeOnMotionData_t;

/**
 * @brief Significant-motion sensitivity configuration.
 */
typedef struct {
  uint8_t sensitivity_level; /**< Sensitivity level, clamped to 0 through 4. */
} ICM42670_SignificantMotionConfig_t;

/**
 * @brief Combined APEX snapshot data.
 *
 * ICM42670_Read_Apex() reads INT_STATUS2 and INT_STATUS3 once, because those
 * registers are read-clear.
 */
typedef struct {
  ICM42670_PedoData_t pedo; /**< Pedometer data and event flags. */
  ICM42670_FreeFallData_t free_fall; /**< Freefall data and event flag. */
  ICM42670_WakeOnMotionData_t wake_on_motion; /**< WOM axis event flags. */
  uint8_t tilt_detected; /**< Nonzero when tilt was reported. */
  uint8_t low_g_detected; /**< Nonzero when low-g was reported. */
  uint8_t significant_motion_detected; /**< Nonzero when SMD was reported. */
} ICM42670_ApexData_t;

/**
 * @brief Initialize the APEX DMP block before enabling tilt/pedometer features.
 *
 * This sets DMP_ODR to 50 Hz, disables DMP power-save for direct polling use,
 * resets APEX memory, and runs the DMP initialization command.
 *
 * @param config Driver configuration with read, write, and delay callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Init_Apex(const ICM42670_Config *config);

/**
 * @brief Pedometer tracks step count and issues a step detect interrupt.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param pedo_config Pedometer setup values.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Enable_Pedo(const ICM42670_Config *config,
                     const ICM42670_PedoConfig_t *pedo_config);

/**
 * @brief Disable pedometer reporting.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Pedo(const ICM42670_Config *config);

/**
 * @brief Read pedometer data and read-clear step status flags.
 *
 * @param config Driver configuration with a read callback.
 * @param pedo_data Destination for pedometer data.
 * @return ICM42670_OK on success, ICM42670_BUSY if data is invalid, otherwise
 * ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Pedo(const ICM42670_Config *config,
                                     ICM42670_PedoData_t *pedo_data);

/**
 * @brief Tilt detection issues an interrupt when the tilt angle exceeds
 * 35 degrees for more than a programmable time.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param tilt_config Tilt setup values.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t
ICM42670_Enable_Tilt(const ICM42670_Config *config,
                     const ICM42670_TiltConfig_t *tilt_config);

/**
 * @brief Disable tilt detection.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Tilt(const ICM42670_Config *config);

/**
 * @brief Read and clear the tilt event flag.
 *
 * @param config Driver configuration with a read callback.
 * @param tilt_detected Destination flag, nonzero when tilt was reported.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Tilt(const ICM42670_Config *config,
                                     uint8_t *tilt_detected);

/**
 * @brief Low-g detection triggers an interrupt when the absolute value of the
 * accelerometer combined axis falls below a programmable threshold and stays
 * below the threshold for a programmable time.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param low_g_config Low-g setup values.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Enable_Low_G(const ICM42670_Config *config,
                      const ICM42670_LowGConfig_t *low_g_config);

/**
 * @brief Disable low-g detection.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Low_G(const ICM42670_Config *config);

/**
 * @brief Read and clear the low-g event flag.
 *
 * @param config Driver configuration with a read callback.
 * @param low_g_detected Destination flag, nonzero when low-g was reported.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Low_G(const ICM42670_Config *config,
                                      uint8_t *low_g_detected);

/**
 * @brief Freefall detection triggers an interrupt when device freefall is
 * detected and outputs freefall duration.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param free_fall_config Freefall setup values.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Enable_Free_Fall(const ICM42670_Config *config,
                          const ICM42670_FreeFallConfig_t *free_fall_config);

/**
 * @brief Disable freefall detection.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Free_Fall(const ICM42670_Config *config);

/**
 * @brief Read freefall data and read-clear the freefall event flag.
 *
 * @param config Driver configuration with a read callback.
 * @param free_fall_data Destination for freefall data.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Free_Fall(const ICM42670_Config *config,
                        ICM42670_FreeFallData_t *free_fall_data);

/**
 * @brief Wake on Motion detects motion when accelerometer samples exceed a
 * programmable threshold. This motion event can be used to enable device
 * operation from sleep mode.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param wake_on_motion_config Wake-on-motion threshold setup.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Enable_Wake_On_Motion(
    const ICM42670_Config *config,
    const ICM42670_WakeOnMotionConfig_t *wake_on_motion_config);

/**
 * @brief Disable wake-on-motion detection.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Wake_On_Motion(const ICM42670_Config *config);

/**
 * @brief Read and clear wake-on-motion axis flags.
 *
 * @param config Driver configuration with a read callback.
 * @param wake_on_motion_data Destination for axis flags.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Wake_On_Motion(const ICM42670_Config *config,
                             ICM42670_WakeOnMotionData_t *wake_on_motion_data);

/**
 * @brief Significant Motion Detector detects significant motion based on
 * accelerometer data.
 *
 * @param config Driver configuration with read and write callbacks.
 * @param significant_motion_config Significant-motion setup values.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR or ICM42670_BUSY.
 */
ICM42670_Status_t ICM42670_Enable_Significant_Motion(
    const ICM42670_Config *config,
    const ICM42670_SignificantMotionConfig_t *significant_motion_config);

/**
 * @brief Disable significant-motion detection.
 *
 * @param config Driver configuration with read and write callbacks.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Disable_Significant_Motion(const ICM42670_Config *config);

/**
 * @brief Read and clear the significant-motion event flag.
 *
 * @param config Driver configuration with a read callback.
 * @param significant_motion_detected Destination flag, nonzero when reported.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Significant_Motion(const ICM42670_Config *config,
                                 uint8_t *significant_motion_detected);

/**
 * @brief Read a combined APEX data snapshot.
 *
 * This is the preferred read path when multiple APEX features are enabled,
 * because INT_STATUS2 and INT_STATUS3 are read-clear.
 *
 * @param config Driver configuration with a read callback.
 * @param apex_data Destination for combined APEX data.
 * @return ICM42670_OK on success, otherwise ICM42670_ERROR.
 */
ICM42670_Status_t ICM42670_Read_Apex(const ICM42670_Config *config,
                                     ICM42670_ApexData_t *apex_data);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_APEX_H */
