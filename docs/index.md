@page project_overview Project Overview

Platform-agnostic C driver for the TDK InvenSense ICM-42670-P IMU, with a
small callback-based core and optional platform adapters for ESP-IDF and STM32
HAL projects.

## Start Here

- Quick setup: use the main **Quick Start** page.
- Core API reference: `ICM42670_driver.h`.
- Optional features: `ICM42670_apex.h`, `ICM42670_fifo.h`, and
  `ICM42670_fsync.h`.
- Platform adapters: `ports/esp_idf` and `ports/stm32_hal`.
- Repository details: [README on GitHub](https://github.com/SleepyPandas/ICM42670_Driver/blob/main/README.md).

## Main Files

| File | Purpose |
| --- | --- |
| `ICM42670_Driver/ICM42670_driver.h` | Core config, init, power, accel, gyro, temperature, and calibration API |
| `ICM42670_Driver/ICM42670_apex.h` | Optional APEX motion features such as pedometer, tilt, WOM, and freefall |
| `ICM42670_Driver/ICM42670_fifo.h` | FIFO setup, packet parsing, flushing, watermark, and interrupt routing |
| `ICM42670_Driver/ICM42670_fsync.h` | Optional FSYNC timestamp capture and event readout |
| `ICM42670_Driver/ports/esp_idf/ICM42670_esp_idf.h` | ESP-IDF I2C/SPI adapter helpers |
| `ICM42670_Driver/ports/stm32_hal/ICM42670_stm32_hal.h` | STM32 HAL I2C/SPI/I3C adapter helpers |

## Driver Shape

The portable core does not include STM32 HAL or ESP-IDF headers. Applications
fill an `ICM42670_Config` with `read_reg`, `write_reg`, `delay_ms`, and a user
handle. Platform adapters only populate those callbacks and keep their platform
state outside the core driver.

## Optional Modules

The optional feature files are intentionally separate from the core API:

- APEX for on-chip motion classification and event detection.
- FIFO for buffered accel/gyro packets and interrupt routing.
- FSYNC for external sync timestamps and event status.

This keeps the basic bring-up path small while still exposing higher-level IMU
features when an application needs them.
