# ICM42670 Portable Driver

Portable C driver for the TDK InvenSense ICM-42670-P IMU.

The reusable driver lives in `ICM42670_Driver/`. It is designed around a small
callback-based transport layer so the same core code can run on ESP32,
STM32, Arduino-style environments, Linux userspace, or another embedded
platform.

## Repository Layout

```text
ICM42670_Driver/
  ICM42670_driver.*          Core accel, gyro, temperature, init API
  ICM42670_apex.*            APEX feature helpers
  ICM42670_fifo.*            FIFO helpers
  ICM42670_fsync.*           FSYNC helpers
  ICM42670_registermap.h     Register definitions
  ports/
    esp_idf/                 ESP-IDF I2C/SPI adapter
    stm32_hal/               STM32 HAL I2C/SPI/I3C adapter
  examples/
    esp_idf/                 ESP-IDF example apps
  CMakeLists.txt             ESP-IDF component build file
  idf_component.yml          ESP Component Registry manifest
```

## Design

- The core driver has no STM32 HAL or ESP-IDF dependency.
- Platform-specific code lives in `ports/`.
- Applications can either use a ready-made port adapter or provide their own
  read/write callbacks.
- Optional features are split into small modules so users can include only what
  they need.

## ESP-IDF Component

The `ICM42670_Driver/` folder is the ESP-IDF component root. Its
`idf_component.yml`, `CMakeLists.txt`, README, license, source files, and
ESP-IDF adapter are the files intended for ESP Component Registry publishing.

See `ICM42670_Driver/README.md` for ESP-IDF quick-start examples.

## STM32 Testing

This driver was brought up and tested with an STM32 project locally. CubeMX,
linker, startup, and generated HAL files are useful for board validation, but
they are intentionally excluded from the reusable driver package.

## Sensor Datasheet
[DataSheet ICM42670](https://d17t6iyxenbwp1.cloudfront.net/s3fs-public/2026-02/ds-000451_icm-42670-p-datasheet_0.pdf?VersionId=IZnZlzqvpHU7XfSMavsmHv4yFZkfwwyd)