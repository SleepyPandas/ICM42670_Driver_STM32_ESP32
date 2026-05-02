/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ICM42670_apex.h"
#include "ICM42670_driver.h"
#include "ports/stm32_hal/ICM42670_stm32_hal.h"
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ICM42670_SPI_CS_GPIO_Port CS_SPI_GPIO_Port
#define ICM42670_SPI_CS_Pin CS_SPI_Pin

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I3C_HandleTypeDef hi3c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I3C1_Init(void);
/* USER CODE BEGIN PFP */
static void UART_SendLine(const char *line);
static void UART_SendScaledSample(const ICM42670_Accel_t *accel,
                                  const ICM42670_Gyro_t *gyro,
                                  int16_t temp_raw, float temp_c);
static void UART_SendWhoAmI(uint8_t who_am_i);
static void UART_SendApexEnableStatus(const char *name,
                                      ICM42670_Status_t status);
static void UART_SendApexData(const ICM42670_PedoData_t *pedo,
                              uint8_t tilt_detected, uint8_t low_g_detected,
                              const ICM42670_FreeFallData_t *free_fall,
                              const ICM42670_WakeOnMotionData_t *wom,
                              uint8_t smd_detected);
static ICM42670_Status_t
ICM42670_EnableInterruptTraceTest(const ICM42670_Config *config);
static void ICM42670_EnableApexSmokeTest(const ICM42670_Config *config);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void UART_SendLine(const char *line) {
  size_t len = 0U;

  if (line == NULL) {
    return;
  }

  while (line[len] != '\0') {
    len++;
  }

  (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len,
                          HAL_MAX_DELAY);
}

static int32_t FloatToScaledInt(float value, float scale) {
  return (int32_t)((value >= 0.0f) ? (value * scale + 0.5f)
                                  : (value * scale - 0.5f));
}

static int32_t AbsInt32(int32_t value) {
  return (value < 0) ? -value : value;
}

static char ScaledSign(int32_t value) { return (value < 0) ? '-' : '+'; }

static void UART_SendScaledSample(const ICM42670_Accel_t *accel,
                                  const ICM42670_Gyro_t *gyro,
                                  int16_t temp_raw, float temp_c) {
  char line[160];
  int32_t accel_x_milli_g = 0;
  int32_t accel_y_milli_g = 0;
  int32_t accel_z_milli_g = 0;
  int32_t gyro_x_milli_dps = 0;
  int32_t gyro_y_milli_dps = 0;
  int32_t gyro_z_milli_dps = 0;
  int32_t temp_centi = FloatToScaledInt(temp_c, 100.0f);
  int32_t temp_whole = temp_centi / 100;
  int32_t temp_frac = temp_centi % 100;

  if ((accel == NULL) || (gyro == NULL)) {
    return;
  }

  accel_x_milli_g = FloatToScaledInt(accel->x_g, 1000.0f);
  accel_y_milli_g = FloatToScaledInt(accel->y_g, 1000.0f);
  accel_z_milli_g = FloatToScaledInt(accel->z_g, 1000.0f);
  gyro_x_milli_dps = FloatToScaledInt(gyro->x_dps, 1000.0f);
  gyro_y_milli_dps = FloatToScaledInt(gyro->y_dps, 1000.0f);
  gyro_z_milli_dps = FloatToScaledInt(gyro->z_dps, 1000.0f);

  if (temp_frac < 0) {
    temp_frac = -temp_frac;
  }

  int len = snprintf(line, sizeof(line),
                     "ACC: X=%c%ld.%03ldg Y=%c%ld.%03ldg Z=%c%ld.%03ldg | "
                     "GYR: X=%c%ld.%03lddps Y=%c%ld.%03lddps "
                     "Z=%c%ld.%03lddps | "
                     "TEMP raw: %d TEMP=%ld.%02ld C\r\n",
                     ScaledSign(accel_x_milli_g),
                     (long)(AbsInt32(accel_x_milli_g) / 1000),
                     (long)(AbsInt32(accel_x_milli_g) % 1000),
                     ScaledSign(accel_y_milli_g),
                     (long)(AbsInt32(accel_y_milli_g) / 1000),
                     (long)(AbsInt32(accel_y_milli_g) % 1000),
                     ScaledSign(accel_z_milli_g),
                     (long)(AbsInt32(accel_z_milli_g) / 1000),
                     (long)(AbsInt32(accel_z_milli_g) % 1000),
                     ScaledSign(gyro_x_milli_dps),
                     (long)(AbsInt32(gyro_x_milli_dps) / 1000),
                     (long)(AbsInt32(gyro_x_milli_dps) % 1000),
                     ScaledSign(gyro_y_milli_dps),
                     (long)(AbsInt32(gyro_y_milli_dps) / 1000),
                     (long)(AbsInt32(gyro_y_milli_dps) % 1000),
                     ScaledSign(gyro_z_milli_dps),
                     (long)(AbsInt32(gyro_z_milli_dps) / 1000),
                     (long)(AbsInt32(gyro_z_milli_dps) % 1000), temp_raw,
                     (long)temp_whole, (long)temp_frac);

  if (len > 0) {
    if ((size_t)len >= sizeof(line)) {
      len = (int)sizeof(line) - 1;
    }

    (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len,
                            HAL_MAX_DELAY);
  }
}

static void UART_SendWhoAmI(uint8_t who_am_i) {
  char line[32];
  int len = snprintf(line, sizeof(line), "WHO_AM_I=0x%02X\r\n", who_am_i);

  if (len > 0) {
    (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len,
                            HAL_MAX_DELAY);
  }
}

static void UART_SendApexEnableStatus(const char *name,
                                      ICM42670_Status_t status) {
  char line[64];
  int len = 0;

  if (name == NULL) {
    return;
  }

  len = snprintf(line, sizeof(line), "APEX enable %-4s: %ld\r\n", name,
                 (long)status);
  if (len > 0) {
    if ((size_t)len >= sizeof(line)) {
      len = (int)sizeof(line) - 1;
    }

    (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len,
                            HAL_MAX_DELAY);
  }
}

static void UART_SendApexData(const ICM42670_PedoData_t *pedo,
                              uint8_t tilt_detected, uint8_t low_g_detected,
                              const ICM42670_FreeFallData_t *free_fall,
                              const ICM42670_WakeOnMotionData_t *wom,
                              uint8_t smd_detected) {
  char line[192];
  int len = 0;

  if ((pedo == NULL) || (free_fall == NULL) || (wom == NULL)) {
    return;
  }

  len = snprintf(line, sizeof(line),
                 "APEX: steps=%u step=%u ovf=%u act=%u tilt=%u lowg=%u "
                 "ff=%u ff_dur=%u wom=%u/%u/%u smd=%u\r\n",
                 (unsigned)pedo->step_count, (unsigned)pedo->step_detected,
                 (unsigned)pedo->overflow, (unsigned)pedo->activity,
                 (unsigned)tilt_detected, (unsigned)low_g_detected,
                 (unsigned)free_fall->detected,
                 (unsigned)free_fall->duration_samples,
                 (unsigned)wom->x_detected, (unsigned)wom->y_detected,
                 (unsigned)wom->z_detected, (unsigned)smd_detected);

  if (len > 0) {
    if ((size_t)len >= sizeof(line)) {
      len = (int)sizeof(line) - 1;
    }

    (void)HAL_UART_Transmit(&huart3, (uint8_t *)line, (uint16_t)len,
                            HAL_MAX_DELAY);
  }
}

static ICM42670_Status_t
ICM42670_EnableInterruptTraceTest(const ICM42670_Config *config) {
  const uint8_t int_config =
      ICM42670_INT_CONFIG_BOTH_LATCHED_PUSH_PULL_ACTIVE_HIGH;
  const uint8_t int_source0 = ICM42670_INT_SOURCE0_DRDY_INT1_EN;
  const uint8_t int_source3 = ICM42670_INT_SOURCE3_DRDY_INT2_EN;

  if ((config == NULL) || (config->write_reg == NULL)) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_INT_CONFIG, &int_config,
                        1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_INT_SOURCE0, &int_source0,
                        1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (config->write_reg(config->handle, ICM42670_REG_INT_SOURCE3, &int_source3,
                        1U) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  HAL_Delay(20U);

  if ((HAL_GPIO_ReadPin(INT1_INPUT_GPIO_Port, INT1_INPUT_Pin) !=
       GPIO_PIN_SET) ||
      (HAL_GPIO_ReadPin(INT2_INPUT_GPIO_Port, INT2_INPUT_Pin) !=
       GPIO_PIN_SET)) {
    return ICM42670_ERROR;
  }

  return ICM42670_OK;
}

static void ICM42670_EnableApexSmokeTest(const ICM42670_Config *config) {
  ICM42670_PedoConfig_t pedo_config = {.slow_walk_enable = 1U};
  ICM42670_TiltConfig_t tilt_config = {.wait_time_s = 2U};
  ICM42670_LowGConfig_t low_g_config = {.threshold_mg = 250U,
                                        .sample_count = 4U};
  ICM42670_FreeFallConfig_t free_fall_config = {.min_distance_cm = 10U,
                                                .max_distance_cm = 102U,
                                                .debounce_ms = 1250U};
  ICM42670_WakeOnMotionConfig_t wom_config = {.x_threshold_mg = 400U,
                                              .y_threshold_mg = 400U,
                                              .z_threshold_mg = 400U};
  ICM42670_SignificantMotionConfig_t smd_config = {.sensitivity_level = 2U};
  ICM42670_Status_t apex_init_status = ICM42670_Init_Apex(config);

  UART_SendApexEnableStatus("INIT", apex_init_status);
  if (apex_init_status != ICM42670_OK) {
    return;
  }

  UART_SendApexEnableStatus("PEDO",
                            ICM42670_Enable_Pedo(config, &pedo_config));
  UART_SendApexEnableStatus("TILT",
                            ICM42670_Enable_Tilt(config, &tilt_config));
  UART_SendApexEnableStatus("LOWG",
                            ICM42670_Enable_Low_G(config, &low_g_config));
  UART_SendApexEnableStatus(
      "FF", ICM42670_Enable_Free_Fall(config, &free_fall_config));
  UART_SendApexEnableStatus("WOM",
                            ICM42670_Enable_Wake_On_Motion(config, &wom_config));
  UART_SendApexEnableStatus(
      "SMD", ICM42670_Enable_Significant_Motion(config, &smd_config));
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_I3C1_Init();
  /* USER CODE BEGIN 2 */
  ICM42670_STM32_SPIBus imu_spi_bus = {0};
  ICM42670_Config imu_config = {
      // .accel_odr = 0,
      .accel_fs = ICM42670_ACCEL_FS_4G,
      // .gyro_odr = 0,
      .gyro_fs = ICM42670_GYRO_FS_500_DPS,
  };
  int16_t temp_raw = 0;
  float temp_c = 0.0f;
  uint8_t who_am_i = 0U;
  uint8_t imu_ready = 0U;
  ICM42670_ApexData_t apex_data = {0};
  ICM42670_Status_t apex_status = ICM42670_ERROR;

  if (ICM42670_STM32_SPI_INIT(

          &imu_config, &imu_spi_bus, &hspi1, ICM42670_SPI_CS_GPIO_Port,
          ICM42670_SPI_CS_Pin

          ) != ICM42670_OK) {
    UART_SendLine("ICM42670 STM32 SPI setup failed\r\n");
  } else {
    HAL_Delay(10);

    if (imu_config.read_reg(imu_config.handle, ICM42670_REG_WHO_AM_I, &who_am_i,
                            1U) == ICM42670_OK) {
      UART_SendWhoAmI(who_am_i);
    } else {
      UART_SendLine("WHO_AM_I read transaction failed\r\n");
    }

    if (ICM42670_Init(&imu_config) == ICM42670_OK) {
      imu_ready = 1U;
      UART_SendLine("ICM42670 init OK\r\n");
      if (ICM42670_EnableInterruptTraceTest(&imu_config) == ICM42670_OK) {
        UART_SendLine("ICM42670 INT1/INT2 trace enabled and GPIO high\r\n");
      } else {
        UART_SendLine("ERROR: ICM42670 INT1/INT2 GPIO not high\r\n");
      }
      ICM42670_EnableApexSmokeTest(&imu_config);
    } else {
      UART_SendLine("ICM42670 init failed\r\n");
    }
  }

  if (imu_ready != 0U) {
    ICM42670_Gyro_Calibration(&imu_config);
  }

  ICM42670_Accel_t accel = {0};
  ICM42670_Gyro_t gyro = {0};

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    if (imu_ready != 0U) {
      if ((ICM42670_ReadAccelG(&imu_config, &accel) == ICM42670_OK) &&
          (ICM42670_ReadGyroDps(&imu_config, &gyro) == ICM42670_OK) &&
          (ICM42670_ReadTempRaw(&imu_config, &temp_raw) == ICM42670_OK) &&
          (ICM42670_ReadTempC(&imu_config, &temp_c) == ICM42670_OK)) {
        UART_SendScaledSample(&accel, &gyro, temp_raw, temp_c);
      } else {
        UART_SendLine("ICM42670 raw read failed\r\n");
      }

      apex_status = ICM42670_Read_Apex(&imu_config, &apex_data);
      if (apex_status == ICM42670_OK) {
        UART_SendApexData(&apex_data.pedo, apex_data.tilt_detected,
                          apex_data.low_g_detected, &apex_data.free_fall,
                          &apex_data.wake_on_motion,
                          apex_data.significant_motion_detected);
      } else {
        UART_SendLine("ICM42670 APEX read failed\r\n");
      }
    }

    HAL_Delay(200);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 5462;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_1);
}

/**
  * @brief I3C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I3C1_Init(void)
{

  /* USER CODE BEGIN I3C1_Init 0 */

  /* USER CODE END I3C1_Init 0 */

  I3C_FifoConfTypeDef sFifoConfig = {0};
  I3C_CtrlConfTypeDef sCtrlConfig = {0};

  /* USER CODE BEGIN I3C1_Init 1 */

  /* USER CODE END I3C1_Init 1 */
  hi3c1.Instance = I3C1;
  hi3c1.Mode = HAL_I3C_MODE_CONTROLLER;
  hi3c1.Init.CtrlBusCharacteristic.SDAHoldTime = HAL_I3C_SDA_HOLD_TIME_0_5;
  hi3c1.Init.CtrlBusCharacteristic.WaitTime = HAL_I3C_OWN_ACTIVITY_STATE_0;
  hi3c1.Init.CtrlBusCharacteristic.SCLPPLowDuration = 0x3c;
  hi3c1.Init.CtrlBusCharacteristic.SCLI3CHighDuration = 0x02;
  hi3c1.Init.CtrlBusCharacteristic.SCLODLowDuration = 0x2c;
  hi3c1.Init.CtrlBusCharacteristic.SCLI2CHighDuration = 0x12;
  hi3c1.Init.CtrlBusCharacteristic.BusFreeDuration = 0x22;
  hi3c1.Init.CtrlBusCharacteristic.BusIdleDuration = 0x3e;
  if (HAL_I3C_Init(&hi3c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure FIFO
  */
  sFifoConfig.RxFifoThreshold = HAL_I3C_RXFIFO_THRESHOLD_1_4;
  sFifoConfig.TxFifoThreshold = HAL_I3C_TXFIFO_THRESHOLD_1_4;
  sFifoConfig.ControlFifo = HAL_I3C_CONTROLFIFO_DISABLE;
  sFifoConfig.StatusFifo = HAL_I3C_STATUSFIFO_DISABLE;
  if (HAL_I3C_SetConfigFifo(&hi3c1, &sFifoConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure controller
  */
  sCtrlConfig.DynamicAddr = 0;
  sCtrlConfig.StallTime = 0x00;
  sCtrlConfig.HotJoinAllowed = DISABLE;
  sCtrlConfig.ACKStallState = DISABLE;
  sCtrlConfig.CCCStallState = DISABLE;
  sCtrlConfig.TxStallState = DISABLE;
  sCtrlConfig.RxStallState = DISABLE;
  sCtrlConfig.HighKeeperSDA = DISABLE;
  if (HAL_I3C_Ctrl_Config(&hi3c1, &sCtrlConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I3C1_Init 2 */

  /* USER CODE END I3C1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_DC_Pin|CS_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ARD_D1_TX_Pin ARD_D0_RX_Pin */
  GPIO_InitStruct.Pin = ARD_D1_TX_Pin|ARD_D0_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DC_Pin CS_SPI_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin|CS_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : INT1_INPUT_Pin */
  GPIO_InitStruct.Pin = INT1_INPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT1_INPUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INT2_INPUT_Pin */
  GPIO_InitStruct.Pin = INT2_INPUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INT2_INPUT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
