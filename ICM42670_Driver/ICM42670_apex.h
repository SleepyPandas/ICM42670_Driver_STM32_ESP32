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



ICM42670_Status_t ICM42670_Enable_Pedo(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Pedo(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Enable_Tilt(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Tilt(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Enable_Low_G(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Low_G(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Enable_Free_Fall(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Free_Fall(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Enable_Wake_On_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Wake_On_Motion(const ICM42670_Config *config);

ICM42670_Status_t ICM42670_Enable_Significant_Motion(const ICM42670_Config *config);
ICM42670_Status_t ICM42670_Disable_Significant_Motion(const ICM42670_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* ICM42670_REGISTERMAP_H */
