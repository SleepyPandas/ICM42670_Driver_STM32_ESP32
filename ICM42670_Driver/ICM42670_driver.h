/**
 * @file ICM42670_driver.h
 * @brief Basic ICM-42670-P driver functions and definitions.
 * 
 *
 * This file defines the main functions for initializing and reading data from the ICM-42670-P IMU chip. 
 *
 */

#ifndef ICM42670_DRIVER_H
#define ICM42670_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif



// Status codes for ICM42670 driver functions.

typedef enum {
  ICM42670_OK = 0,
  ICM42670_ERROR = -1,
  ICM42670_BUSY = -2,
} ICM42670_StatusTypeDef;


#ifdef __cplusplus
}
#endif

#endif /* ICM42670_REGISTERMAP_H */
