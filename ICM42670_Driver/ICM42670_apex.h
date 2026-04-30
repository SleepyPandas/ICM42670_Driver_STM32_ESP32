/**
 * @file ICM42670_apex.h
 * @brief ICM-42670-P APEX feature register addresses and field values.
 * 
 *
 * This file defines the register addresses and field values for the APEX features of the ICM-42670-P IMU chip, including Activity Recognition, Gestures, and Step Counter. These features can be configured and accessed through the registers defined in this header.
 *
 */

#ifndef ICM42670_APEX_H
#define ICM42670_APEX_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Pedometer tracks step count and issues a step detect interrupt.
 */
ICM42670_Status_t ICM42670_Enable_Pedo(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Pedo(const ICM42670_Config *config);

/**
 * @brief Tilt detection issues an interrupt when the tilt angle exceeds
 * 35 degrees for more than a programmable time.
 */
ICM42670_Status_t ICM42670_Enable_Tilt(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Tilt(const ICM42670_Config *config);

/**
 * @brief Low-g detection triggers an interrupt when the absolute value of the
 * accelerometer combined axis falls below a programmable threshold and stays
 * below the threshold for a programmable time.
 */
ICM42670_Status_t ICM42670_Enable_Low_G(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Low_G(const ICM42670_Config *config);

/**
 * @brief Freefall detection triggers an interrupt when device freefall is
 * detected and outputs freefall duration.
 */
ICM42670_Status_t ICM42670_Enable_Free_Fall(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Free_Fall(const ICM42670_Config *config);

/**
 * @brief Wake on Motion detects motion when accelerometer samples exceed a
 * programmable threshold. This motion event can be used to enable device
 * operation from sleep mode.
 */
ICM42670_Status_t ICM42670_Enable_Wake_On_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Wake_On_Motion(const ICM42670_Config *config);

/**
 * @brief Significant Motion Detector detects significant motion based on
 * accelerometer data.
 */
ICM42670_Status_t ICM42670_Enable_Significant_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Significant_Motion(const ICM42670_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_REGISTERMAP_H */
