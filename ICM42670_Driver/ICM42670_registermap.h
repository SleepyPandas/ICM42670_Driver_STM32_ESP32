/**
 * @file ST7789V3.h
 * @brief Public API for the ST7789V3 SPI display driver.
 *
 * This module provides a small, platform-agnostic interface for controlling an
 * ST7789V3-based display over SPI. The application supplies the hardware
 * access callbacks in ::ST7789V3_Config, then uses the driver to initialize
 * the display, configure core display settings, draw graphics primitives, and
 * optionally stream pixel buffers with DMA.
 *
 * Typical responsibilities handled by this header include:
 * - Display initialization and reset flow
 * - Display mode control such as sleep, inversion, and rotation
 * - Drawing text, pixels, lines, rectangles, and circles
 * - Optional non-blocking DMA pixel transfers with completion callbacks
 */

#ifndef ICM42670_registermap_H
#define ICM42670_registermap_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>














#ifdef __cplusplus
}
#endif



#endif /* __ICM42670_registermap_H */