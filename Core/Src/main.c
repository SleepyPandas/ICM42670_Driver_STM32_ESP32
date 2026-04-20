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
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ICM42670_REG_WHO_AM_I 0x75U
#define ICM42670_SPI_CS_GPIO_Port CS_SPI_GPIO_Port
#define ICM42670_SPI_CS_Pin CS_SPI_Pin
#define ICM42670_I2C_ADDR_LOW (0x68U << 1)
#define ICM42670_I2C_ADDR_HIGH (0x69U << 1)
#define ICM42670_EXPECTED_WHO_AM_I 0x67U
#define ICM42670_REG_TEMP_DATA1 0x09U
#define ICM42670_REG_ACCEL_DATA_X1 0x0BU
#define ICM42670_REG_ACCEL_DATA_Y1 0x0DU
#define ICM42670_REG_ACCEL_DATA_Z1 0x0FU
#define ICM42670_REG_GYRO_DATA_X1 0x11U
#define ICM42670_REG_GYRO_DATA_Y1 0x13U
#define ICM42670_REG_GYRO_DATA_Z1 0x15U
#define ICM42670_REG_PWR_MGMT0 0x1FU
#define ICM42670_PWR_ACCEL_GYRO_LN 0x0FU

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static int16_t ICM42670_CombineBytes(uint8_t msb, uint8_t lsb)
{
  return (int16_t)(((uint16_t)msb << 8U) | (uint16_t)lsb);
}

static HAL_StatusTypeDef ICM42670_ReadInt16(I2C_HandleTypeDef *hi2c,
                                            uint8_t dev_addr,
                                            uint8_t reg_addr,
                                            int16_t *out)
{
  HAL_StatusTypeDef status;
  uint8_t raw[2] = {0};

  status = HAL_I2C_Mem_Read(hi2c, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT,
                            raw, 2, HAL_MAX_DELAY);
  if (status == HAL_OK) {
    *out = ICM42670_CombineBytes(raw[0], raw[1]);
  }

  return status;
}

int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;
  (void)HAL_UART_Transmit(&huart3, &c, 1, HAL_MAX_DELAY);
  return ch;
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
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_StatusTypeDef i2c_status;
  uint8_t who_value_i2c = 0;
  uint8_t active_i2c_addr = ICM42670_I2C_ADDR_LOW;
  uint8_t sensor_cfg_done = 0;
  int16_t temp_raw;
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    who_value_i2c = 0;

    HAL_GPIO_WritePin(ICM42670_SPI_CS_GPIO_Port, ICM42670_SPI_CS_Pin,
                      GPIO_PIN_SET);

    i2c_status = HAL_I2C_Mem_Read(&hi2c1, ICM42670_I2C_ADDR_LOW,
                                  ICM42670_REG_WHO_AM_I,
                                  I2C_MEMADD_SIZE_8BIT, &who_value_i2c, 1,
                                  HAL_MAX_DELAY);

    if (i2c_status != HAL_OK) {
      i2c_status = HAL_I2C_Mem_Read(&hi2c1, ICM42670_I2C_ADDR_HIGH,
                                    ICM42670_REG_WHO_AM_I,
                                    I2C_MEMADD_SIZE_8BIT, &who_value_i2c, 1,
                                    HAL_MAX_DELAY);
      if (i2c_status == HAL_OK) {
        active_i2c_addr = ICM42670_I2C_ADDR_HIGH;
      }
    } else {
      active_i2c_addr = ICM42670_I2C_ADDR_LOW;
    }

    if (i2c_status == HAL_OK) {
      if (who_value_i2c == ICM42670_EXPECTED_WHO_AM_I) {
        if (sensor_cfg_done == 0U) {
          uint8_t pwr_cfg = ICM42670_PWR_ACCEL_GYRO_LN;
          HAL_StatusTypeDef cfg_status = HAL_I2C_Mem_Write(
              &hi2c1, active_i2c_addr, ICM42670_REG_PWR_MGMT0,
              I2C_MEMADD_SIZE_8BIT, &pwr_cfg, 1, HAL_MAX_DELAY);

          if (cfg_status == HAL_OK) {
            sensor_cfg_done = 1U;
            printf("ICM42670 detected at 0x%02X, streaming raw values over UART\r\n",
                   active_i2c_addr >> 1);
            HAL_Delay(50);
          } else {
            printf("ICM42670 config write failed (status=%d)\r\n", cfg_status);
            continue;
          }
        }

        i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                        ICM42670_REG_TEMP_DATA1, &temp_raw);
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_ACCEL_DATA_X1,
                                          &accel_x);
        }
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_ACCEL_DATA_Y1,
                                          &accel_y);
        }
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_ACCEL_DATA_Z1,
                                          &accel_z);
        }
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_GYRO_DATA_X1,
                                          &gyro_x);
        }
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_GYRO_DATA_Y1,
                                          &gyro_y);
        }
        if (i2c_status == HAL_OK) {
          i2c_status = ICM42670_ReadInt16(&hi2c1, active_i2c_addr,
                                          ICM42670_REG_GYRO_DATA_Z1,
                                          &gyro_z);
        }

        if (i2c_status == HAL_OK) {
          printf("WHO=0x%02X T=%d AX=%d AY=%d AZ=%d GX=%d GY=%d GZ=%d\r\n",
                 who_value_i2c, temp_raw, accel_x, accel_y, accel_z, gyro_x,
                 gyro_y, gyro_z);
        } else {
          printf("I2C frame read failed (status=%d)\r\n", i2c_status);
        }
      } else {
        printf("Unexpected WHO_AM_I: 0x%02X\r\n", who_value_i2c);
      }
    } else {
      printf("I2C WHO_AM_I read failed on 0x68 and 0x69 (status=%d)\r\n",
             i2c_status);
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Continuous SPI traffic for logic analyzer capture. */
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00300B28;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  if (HAL_I2CEx_ConfigFastModePlus(&hi2c1, I2C_FASTMODEPLUS_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
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
  HAL_GPIO_WritePin(GPIOC, LCD_DC_Pin|LCD_RES_Pin|CS_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ARD_D1_TX_Pin ARD_D0_RX_Pin */
  GPIO_InitStruct.Pin = ARD_D1_TX_Pin|ARD_D0_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DC_Pin LCD_RES_Pin CS_SPI_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin|LCD_RES_Pin|CS_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
