# ICM42670 Platform Agnostic Driver (Tested On STM32)

## Goal-> Provide a Very Simple API for ESP32, STM32, Arduino, Raspberrypi
Should be able to call a couple of functions and simple pointers to get started to poll data accel, temp, gyro, and All APEX functions 

After there should be more complex configurations if you need them for example: self-test registers, FIFO, Power Modes, Noise Filtering, calibration etc.


~~1. Enable abstract or function pointers for platform agnostic use~~ 

~~1.1 Expose I2C, SPI and Optionally I3C~~

~~2. Configure Power Modes~~ 

~~2.1 Configure Full Scale Range for Gyro And Accel~~ 

~~2.2 Configure ODR Output Data Rates~~ 

    2.3 Configure LOW PASS FILTERS (Not MAJOR)

3. FIFO Buffer Configurations i.e With Apex and Without apex for 2.25 KByte FIFO buffer 1KB FIFO if Apex is active 

    3.1 Expose Read from Register for Gyro/Accel and also read from FIFO buffer 
4. Interrupt Routing for INT1 / INT2 in conjunction with Apex READY and DATA ready 

~~5. Expose Apex functions in simple abstract layer~~  

6. Expose a function to use FSYNC 




[DataSheet STM32](https://www.st.com/resource/en/user_manual/um3121-stm32h5-nucleo64-board-mb1814-stmicroelectronics.pdf)
[DataSheet ICM42670](https://d17t6iyxenbwp1.cloudfront.net/s3fs-public/2026-02/ds-000451_icm-42670-p-datasheet_0.pdf?VersionId=IZnZlzqvpHU7XfSMavsmHv4yFZkfwwyd)