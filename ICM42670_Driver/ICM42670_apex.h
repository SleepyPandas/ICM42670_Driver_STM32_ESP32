/**
 * @file ICM42670_apex.h
 * @brief ICM-42670-P APEX feature register addresses and field values.
 *
 * This file defines the register addresses and field values for the APEX
 * features of the ICM-42670-P IMU chip, including Activity Recognition,
 * Gestures, and Step Counter. These features can be configured and accessed
 * through the registers defined in this header.
 *
 */

#ifndef ICM42670_APEX_H
#define ICM42670_APEX_H

#include "ICM42670_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ICM42670_APEX_ACTIVITY_UNKNOWN = 0U,
  ICM42670_APEX_ACTIVITY_WALK = 1U,
  ICM42670_APEX_ACTIVITY_RUN = 2U,
} ICM42670_ApexActivity_t;

typedef struct {
  uint8_t slow_walk_enable;
} ICM42670_PedoConfig_t;

typedef struct {
  uint16_t step_count;
  uint8_t cadence_raw;
  ICM42670_ApexActivity_t activity;
  uint8_t step_detected;
  uint8_t overflow;
} ICM42670_PedoData_t;

typedef struct {
  uint8_t wait_time_s;
} ICM42670_TiltConfig_t;

typedef struct {
  uint16_t threshold_mg;
  uint8_t sample_count;
} ICM42670_LowGConfig_t;

typedef struct {
  uint16_t min_distance_cm;
  uint16_t max_distance_cm;
  uint16_t debounce_ms;
} ICM42670_FreeFallConfig_t;

typedef struct {
  uint8_t detected;
  uint16_t duration_samples;
} ICM42670_FreeFallData_t;

typedef struct {
  uint16_t x_threshold_mg;
  uint16_t y_threshold_mg;
  uint16_t z_threshold_mg;
} ICM42670_WakeOnMotionConfig_t;

typedef struct {
  uint8_t x_detected;
  uint8_t y_detected;
  uint8_t z_detected;
} ICM42670_WakeOnMotionData_t;

typedef struct {
  uint8_t sensitivity_level;
} ICM42670_SignificantMotionConfig_t;

typedef struct {
  ICM42670_PedoData_t pedo;
  ICM42670_FreeFallData_t free_fall;
  ICM42670_WakeOnMotionData_t wake_on_motion;
  uint8_t tilt_detected;
  uint8_t low_g_detected;
  uint8_t significant_motion_detected;
} ICM42670_ApexData_t;

/**
 * @brief Initialize the APEX DMP block before enabling tilt/pedometer features.
 *
 * This sets DMP_ODR to 50 Hz, disables DMP power-save for direct polling use,
 * resets APEX memory, and runs the DMP initialization command.
 */
ICM42670_Status_t ICM42670_Init_Apex(const ICM42670_Config *config);

/**
 * @brief Pedometer tracks step count and issues a step detect interrupt.
 */
ICM42670_Status_t ICM42670_Enable_Pedo(const ICM42670_Config *config,
                     const ICM42670_PedoConfig_t *pedo_config);

ICM42670_Status_t ICM42670_Disable_Pedo(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Read_Pedo(const ICM42670_Config *config,
                                     ICM42670_PedoData_t *pedo_data);

/**
 * @brief Tilt detection issues an interrupt when the tilt angle exceeds
 * 35 degrees for more than a programmable time.
 */
ICM42670_Status_t
ICM42670_Enable_Tilt(const ICM42670_Config *config,
                     const ICM42670_TiltConfig_t *tilt_config);
ICM42670_Status_t ICM42670_Disable_Tilt(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Read_Tilt(const ICM42670_Config *config,
                                     uint8_t *tilt_detected);

/**
 * @brief Low-g detection triggers an interrupt when the absolute value of the
 * accelerometer combined axis falls below a programmable threshold and stays
 * below the threshold for a programmable time.
 */
ICM42670_Status_t ICM42670_Enable_Low_G(const ICM42670_Config *config,
                      const ICM42670_LowGConfig_t *low_g_config);
ICM42670_Status_t ICM42670_Disable_Low_G(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Read_Low_G(const ICM42670_Config *config,
                                      uint8_t *low_g_detected);

/**
 * @brief Freefall detection triggers an interrupt when device freefall is
 * detected and outputs freefall duration.
 */
ICM42670_Status_t ICM42670_Enable_Free_Fall(const ICM42670_Config *config,
                          const ICM42670_FreeFallConfig_t *free_fall_config);

ICM42670_Status_t ICM42670_Disable_Free_Fall(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Read_Free_Fall(const ICM42670_Config *config,
                        ICM42670_FreeFallData_t *free_fall_data);

/**
 * @brief Wake on Motion detects motion when accelerometer samples exceed a
 * programmable threshold. This motion event can be used to enable device
 * operation from sleep mode.
 */
ICM42670_Status_t ICM42670_Enable_Wake_On_Motion(
    const ICM42670_Config *config,
    const ICM42670_WakeOnMotionConfig_t *wake_on_motion_config);
ICM42670_Status_t ICM42670_Disable_Wake_On_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Read_Wake_On_Motion(const ICM42670_Config *config,
                             ICM42670_WakeOnMotionData_t *wake_on_motion_data);

/**
 * @brief Significant Motion Detector detects significant motion based on
 * accelerometer data.
 */
ICM42670_Status_t ICM42670_Enable_Significant_Motion(
    const ICM42670_Config *config,
    const ICM42670_SignificantMotionConfig_t *significant_motion_config);
ICM42670_Status_t ICM42670_Disable_Significant_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Read_Significant_Motion(const ICM42670_Config *config,
                                 uint8_t *significant_motion_detected);

ICM42670_Status_t ICM42670_Read_Apex(const ICM42670_Config *config,
                                     ICM42670_ApexData_t *apex_data);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_APEX_H */
