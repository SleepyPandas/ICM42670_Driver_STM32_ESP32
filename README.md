<a href="https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/ds-000451-icm-42670-p.pdf"><img src="https://img.shields.io/badge/Datasheet-ICM--42670--P-0078D4?style=flat-round&logo=bookstack&logoColor=white" alt="Datasheet"/></a> 
![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white) 
![STM32](https://img.shields.io/badge/STM32-03234B?style=flat&logo=stmicroelectronics&logoColor=white) 
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=flat&logo=espressif&logoColor=white) 
![I/O](https://img.shields.io/badge/I%2FO-I3C_%7C_I2C_%7C_SPI-f7b80a)

# ICM42670 Portable Driver

Portable C driver for the TDK InvenSense ICM-42670-P IMU. The reusable core is
built around a small callback-based transport layer so the same sensor logic can
run on ESP-IDF, STM32 HAL, Arduino-style environments, Linux userspace, or
another embedded platform.

## Documentation

[Documentation](https://sleepypandas.github.io/ICM42670_Driver_STM32_ESP32/)

---

## Key Features

| Feature | Implementation |
| :--- | :--- |
| **Platform-Agnostic Driver Design** | `ICM42670_Config` injects register read/write callbacks, a delay callback, and user bus state; the core driver has no STM32 HAL or ESP-IDF dependency |
| **ESP-IDF Adapter** | Ready-made I2C and SPI helpers create or attach ESP-IDF bus/device handles and populate the portable config |
| **STM32 HAL Adapter** | SPI, I2C, and I3C helper paths map HAL handles to the same core callback contract |
| **Scaled Sensor Reads** | Accel, gyro, and temperature APIs expose human-readable `g`, `dps`, and Celsius values while raw-count reads remain available |
| **Explicit Calibration** | `ICM42670_Gyro_Calibration()` estimates stationary gyro bias and stores raw offsets in the config |
| **Optional APEX Module** | Pedometer, tilt, low-g, freefall, wake-on-motion, significant-motion, and combined read-clear snapshot support live outside the core file |
| **Optional FIFO Module** | Simple FIFO setup, packet parsing, watermarking, flushing, and interrupt routing are kept in a focused feature module |
| **Optional FSYNC Module** | External sync edge capture, timestamp delta readout, tag selection, and interrupt routing are available without bloating the base API |

---

## Project Structure

```text
ICM42670_Driver/
├── ICM42670_Driver/
│   ├── ICM42670_driver.c/.h          # Core init, power, accel, gyro, temperature API
│   ├── ICM42670_apex.c/.h            # Optional APEX motion features
│   ├── ICM42670_fifo.c/.h            # Optional FIFO helpers
│   ├── ICM42670_fsync.c/.h           # Optional FSYNC helpers
│   ├── ICM42670_registermap.h        # Register and bit definitions
│   ├── ports/
│   │   ├── esp_idf/                  # ESP-IDF I2C/SPI adapter
│   │   └── stm32_hal/                # STM32 HAL I2C/SPI/I3C adapter
│   └── examples/esp_idf/basic_i2c/   # ESP-IDF bring-up example
├── docs/                             # Doxygen source pages and theme
├── Doxyfile                          # Doxygen configuration
└── .github/workflows/docs.yml        # GitHub Pages documentation build
```

* [Core driver](ICM42670_Driver/ICM42670_driver.c)
* [Core public API](ICM42670_Driver/ICM42670_driver.h)
* [ESP-IDF example](ICM42670_Driver/examples/esp_idf/basic_i2c/main/main.c)

---

### 1. Callback Transport Layer

The core driver never calls a platform SDK directly. Applications provide the
register transport and delay functions through `ICM42670_Config`.

```c
typedef struct {
  ICM42670_Odr_t accel_odr;
  ICM42670_AccelFS_t accel_fs;
  ICM42670_Odr_t gyro_odr;
  ICM42670_GyroFS_t gyro_fs;
  ICM42670_Gyro_Offsets_t gyro_offsets;

  void *handle;
  int8_t (*read_reg)(void *handle, uint8_t reg_addr, uint8_t *data,
                     uint16_t len);
  int8_t (*write_reg)(void *handle, uint8_t reg_addr, const uint8_t *data,
                      uint16_t len);
  void (*delay_ms)(uint32_t ms);
} ICM42670_Config;
```

That contract is the portability boundary: the driver owns register behavior and
sensor math, while the application or adapter owns bus details.

### 2. Initialization Flow

`ICM42670_Init()` validates callbacks, checks `WHO_AM_I`, normalizes invalid
range/ODR fields to driver defaults, writes accel/gyro configuration, and enters
6-axis low-noise mode.

```c
ICM42670_Config imu = {
    .accel_odr = ICM42670_ODR_100_HZ,
    .accel_fs = ICM42670_ACCEL_FS_4G,
    .gyro_odr = ICM42670_ODR_100_HZ,
    .gyro_fs = ICM42670_GYRO_FS_500_DPS,
    .handle = &bus,
    .read_reg = app_read_reg,
    .write_reg = app_write_reg,
    .delay_ms = app_delay_ms,
};

if (ICM42670_Init(&imu) != ICM42670_OK) {
  /* Check wiring, bus mode, address, and WHO_AM_I. */
}
```

### 3. ESP-IDF Component Path

For a normal ESP-IDF project, install the published component:

```sh
idf.py add-dependency "sleepypandas/icm42670_driver^0.1.2"
idf.py reconfigure
```

The ESP-IDF adapter can create a bus/device or attach to handles your
application already owns.

```c
ICM42670_ESP_I2CBus bus = {0};

esp_err_t err = ICM42670_ESP_I2C_Init(&imu, &bus, I2C_NUM_0, GPIO_NUM_21,
                                      GPIO_NUM_22, 0x68, 400000);
if (err == ESP_OK) {
  (void)ICM42670_Init(&imu);
}
```

### 4. STM32 HAL Path

The STM32 adapter keeps HAL details in `ports/stm32_hal` and fills the same
portable config.

```c
static ICM42670_Config imu = {
    .accel_odr = ICM42670_ODR_100_HZ,
    .accel_fs = ICM42670_ACCEL_FS_4G,
    .gyro_odr = ICM42670_ODR_100_HZ,
    .gyro_fs = ICM42670_GYRO_FS_500_DPS,
};
static ICM42670_STM32_SPIBus imu_spi = {0};

ICM42670_STM32_SPI_INIT(&imu, &imu_spi, &hspi1, IMU_CS_GPIO_Port, IMU_CS_Pin);
ICM42670_Init(&imu);
```

### 5. Sensor Read Path

Use scaled reads for application logic and raw reads for debugging or custom
filtering.

```c
ICM42670_Accel_t accel = {0};
ICM42670_Gyro_t gyro = {0};
float temp_c = 0.0f;

ICM42670_ReadAccelG(&imu, &accel);
ICM42670_ReadGyroDps(&imu, &gyro);
ICM42670_ReadTempC(&imu, &temp_c);
```

### 6. Optional Feature Modules

Optional subsystems stay in separate files so the base driver remains small.

```c
#include "ICM42670_apex.h"
#include "ICM42670_fifo.h"
#include "ICM42670_fsync.h"
```

APEX features should use `ICM42670_Read_Apex()` when multiple event sources are
enabled because the status registers are read-clear. FIFO and FSYNC helpers can
be used independently of APEX.

---

## Build And Docs

```sh
# Generate local API docs
doxygen Doxyfile

# Run host tests when available in the checkout
cmake -S tests/host -B build/host-tests
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

The GitHub Pages workflow builds the Doxygen site from source on pushes to
`main`; generated HTML stays out of the repository.

## References

- [ICM-42670-P Datasheet](https://d17t6iyxenbwp1.cloudfront.net/s3fs-public/2026-02/ds-000451_icm-42670-p-datasheet_0.pdf?VersionId=IZnZlzqvpHU7XfSMavsmHv4yFZkfwwyd)
- [ESP Component Registry](https://components.espressif.com/components/sleepypandas/icm42670_driver)
- [Doxygen Awesome CSS](https://github.com/jothepro/doxygen-awesome-css)

---

**Driver Source:** [`ICM42670_driver.c`](ICM42670_Driver/ICM42670_driver.c) |
[`ICM42670_driver.h`](ICM42670_Driver/ICM42670_driver.h) |
**Example:** [`basic_i2c/main.c`](ICM42670_Driver/examples/esp_idf/basic_i2c/main/main.c)
