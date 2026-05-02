/**
 *
 * @file        ICM42670_apex.c
 * @brief ICM-42670-P APEX feature functions.
 *
 * @author Anthony / SleepPandas
 */

#include "ICM42670_apex.h"
#include "ICM42670_internal.h"
#include <stddef.h>

#define ICM42670_APEX_MEM_RESET_DELAY_MS 1U
#define ICM42670_APEX_INIT_DELAY_MS 50U

#define ICM42670_REG_APEX_DATA4 0x1DU
#define ICM42670_REG_APEX_DATA0 0x31U

#define ICM42670_MREG1_APEX_CONFIG3 0x45U
#define ICM42670_MREG1_APEX_CONFIG5 0x47U
#define ICM42670_MREG1_APEX_CONFIG9 0x48U
#define ICM42670_MREG1_APEX_CONFIG10 0x49U
#define ICM42670_MREG1_ACCEL_WOM_X_THR 0x4BU
#define ICM42670_MREG1_ACCEL_WOM_Y_THR 0x4CU
#define ICM42670_MREG1_ACCEL_WOM_Z_THR 0x4DU
#define ICM42670_MREG1_APEX_CONFIG12 0x67U

#define ICM42670_APEX_CONFIG1_PED_ENABLE 0x08U
#define ICM42670_APEX_CONFIG1_TILT_ENABLE 0x10U
#define ICM42670_APEX_CONFIG1_FF_ENABLE 0x20U
#define ICM42670_APEX_CONFIG1_SMD_ENABLE 0x40U

#define ICM42670_APEX_CONFIG0_DMP_MEM_RESET_EN 0x01U
#define ICM42670_APEX_CONFIG0_DMP_INIT_EN 0x04U
#define ICM42670_APEX_CONFIG0_DMP_POWER_SAVE_EN 0x08U
#define ICM42670_APEX_CONFIG1_DMP_ODR_MASK 0x03U
#define ICM42670_APEX_CONFIG1_DMP_ODR_50HZ 0x02U
#define ICM42670_APEX_CONFIG1_FEATURE_ENABLE_MASK 0x78U

#define ICM42670_WOM_CONFIG_WOM_EN 0x01U

#define ICM42670_INT_STATUS2_WOM_Z 0x01U
#define ICM42670_INT_STATUS2_WOM_Y 0x02U
#define ICM42670_INT_STATUS2_WOM_X 0x04U
#define ICM42670_INT_STATUS2_SMD 0x08U

#define ICM42670_INT_STATUS3_LOWG 0x02U
#define ICM42670_INT_STATUS3_FF 0x04U
#define ICM42670_INT_STATUS3_TILT 0x08U
#define ICM42670_INT_STATUS3_STEP_OVF 0x10U
#define ICM42670_INT_STATUS3_STEP_DET 0x20U

#define ICM42670_APEX_CONFIG3_PED_STEP_CNT_TH_MASK 0x0FU
#define ICM42670_APEX_CONFIG3_PED_STEP_CNT_TH_ONE_STEP 0x01U
#define ICM42670_APEX_CONFIG5_TILT_WAIT_MASK 0xC0U
#define ICM42670_APEX_CONFIG9_FF_DEBOUNCE_MASK 0xF0U
#define ICM42670_APEX_CONFIG9_SMD_SENSITIVITY_MASK 0x0EU
#define ICM42670_APEX_CONFIG9_PED_SLOW_WALK_MASK 0x01U
#define ICM42670_APEX_CONFIG10_LOWG_MASK 0xFFU
#define ICM42670_APEX_CONFIG12_FF_DURATION_MASK 0xFFU

typedef enum {
  ICM42670_APEX_DISABLE = 0U,
  ICM42670_APEX_ENABLE = 1U,
} ICM42670_ApexState_t;

static const uint16_t ICM42670_TiltWaitSeconds[] = {0U, 2U, 4U, 6U};

static const uint16_t ICM42670_LowGThresholdMg[] = {
    31U,  63U,  94U,  125U, 156U, 188U, 219U, 250U,
    281U, 313U, 344U, 375U, 406U, 438U, 469U, 500U,
    531U, 563U, 594U, 625U, 656U, 688U, 719U, 750U,
    781U, 813U, 844U, 875U, 906U, 938U, 969U, 1000U};

static const uint16_t ICM42670_FreeFallMinDistanceCm[] = {
    10U, 12U, 13U, 16U, 18U, 20U, 23U, 25U,
    28U, 31U, 34U, 38U, 41U, 45U, 48U, 52U};

static const uint16_t ICM42670_FreeFallMaxDistanceCm[] = {
    102U, 120U, 139U, 159U, 181U, 204U, 228U, 254U,
    281U, 310U, 339U, 371U, 403U, 438U, 473U, 510U};

static const uint16_t ICM42670_FreeFallDebounceMs[] = {
    0U,    1250U, 1375U, 1500U, 1625U, 1750U, 1875U, 2000U,
    2125U, 2250U, 2375U, 2500U, 2625U, 2750U, 2875U, 3000U};

static uint8_t SelectNearestIndex(const uint16_t *values, uint8_t count,
                                  uint16_t target) {
  uint8_t best_index = 0U;
  uint32_t best_diff = 0xFFFFFFFFUL;

  if ((values == NULL) || (count == 0U)) {
    return 0U;
  }

  for (uint8_t i = 0U; i < count; i++) {
    uint32_t diff = (target > values[i]) ? ((uint32_t)target - values[i])
                                         : ((uint32_t)values[i] - target);
    if (diff < best_diff) {
      best_diff = diff;
      best_index = i;
    }
  }

  return best_index;
}

static uint8_t ClampU8(uint8_t value, uint8_t min_value, uint8_t max_value) {
  if (value < min_value) {
    return min_value;
  }

  if (value > max_value) {
    return max_value;
  }

  return value;
}

static uint8_t ConvertWomThresholdMg(uint16_t threshold_mg) {
  uint32_t code = (((uint32_t)threshold_mg * 256UL) + 500UL) / 1000UL;

  return (code > 255UL) ? 255U : (uint8_t)code;
}

static ICM42670_Status_t SetApexConfig1Feature(
    const ICM42670_Config *config, uint8_t mask, ICM42670_ApexState_t state) {
  uint8_t field_value = (state == ICM42670_APEX_ENABLE) ? mask : 0U;

  return ICM42670_UpdateRegBits(config, ICM42670_REG_APEX_CONFIG1, mask,
                                field_value);
}

static ICM42670_Status_t SetWomFeature(const ICM42670_Config *config,
                                       ICM42670_ApexState_t state) {
  uint8_t field_value =
      (state == ICM42670_APEX_ENABLE) ? ICM42670_WOM_CONFIG_WOM_EN : 0U;

  return ICM42670_UpdateRegBits(config, ICM42670_REG_WOM_CONFIG,
                                ICM42670_WOM_CONFIG_WOM_EN, field_value);
}

ICM42670_Status_t ICM42670_Init_Apex(const ICM42670_Config *config) {
  const uint8_t apex_config0_command_mask =
      (uint8_t)(ICM42670_APEX_CONFIG0_DMP_MEM_RESET_EN |
                ICM42670_APEX_CONFIG0_DMP_INIT_EN |
                ICM42670_APEX_CONFIG0_DMP_POWER_SAVE_EN);
  const uint8_t apex_config1_init_mask =
      (uint8_t)(ICM42670_APEX_CONFIG1_FEATURE_ENABLE_MASK |
                ICM42670_APEX_CONFIG1_DMP_ODR_MASK);
  ICM42670_Status_t status;

  if (ICM42670_ValidateMregConfig(config) != ICM42670_OK) {
    return ICM42670_ERROR;
  }

  status = ICM42670_UpdateRegBits(config, ICM42670_REG_APEX_CONFIG1,
                                  apex_config1_init_mask,
                                  ICM42670_APEX_CONFIG1_DMP_ODR_50HZ);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_UpdateRegBits(config, ICM42670_REG_APEX_CONFIG0,
                                  apex_config0_command_mask, 0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_UpdateRegBits(
      config, ICM42670_REG_APEX_CONFIG0,
      ICM42670_APEX_CONFIG0_DMP_MEM_RESET_EN,
      ICM42670_APEX_CONFIG0_DMP_MEM_RESET_EN);
  if (status != ICM42670_OK) {
    return status;
  }

  config->delay_ms(ICM42670_APEX_MEM_RESET_DELAY_MS);

  status = ICM42670_UpdateRegBits(
      config, ICM42670_REG_APEX_CONFIG0,
      ICM42670_APEX_CONFIG0_DMP_MEM_RESET_EN, 0U);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_UpdateRegBits(config, ICM42670_REG_APEX_CONFIG0,
                                  ICM42670_APEX_CONFIG0_DMP_INIT_EN,
                                  ICM42670_APEX_CONFIG0_DMP_INIT_EN);
  if (status != ICM42670_OK) {
    return status;
  }

  config->delay_ms(ICM42670_APEX_INIT_DELAY_MS);

  return ICM42670_UpdateRegBits(config, ICM42670_REG_APEX_CONFIG0,
                                ICM42670_APEX_CONFIG0_DMP_INIT_EN, 0U);
}

ICM42670_Status_t
ICM42670_Enable_Pedo(const ICM42670_Config *config,
                     const ICM42670_PedoConfig_t *pedo_config) {
  ICM42670_Status_t status;
  uint8_t slow_walk_value;

  if (pedo_config == NULL) {
    return ICM42670_ERROR;
  }

  slow_walk_value = (pedo_config->slow_walk_enable != 0U)
                        ? ICM42670_APEX_CONFIG9_PED_SLOW_WALK_MASK
                        : 0U;

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG9,
      ICM42670_APEX_CONFIG9_PED_SLOW_WALK_MASK, slow_walk_value);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG3,
      ICM42670_APEX_CONFIG3_PED_STEP_CNT_TH_MASK,
      ICM42670_APEX_CONFIG3_PED_STEP_CNT_TH_ONE_STEP);
  if (status != ICM42670_OK) {
    return status;
  }

  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_PED_ENABLE,
                               ICM42670_APEX_ENABLE);
}

ICM42670_Status_t ICM42670_Disable_Pedo(const ICM42670_Config *config) {
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_PED_ENABLE,
                               ICM42670_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_Read_Pedo(const ICM42670_Config *config,
                                     ICM42670_PedoData_t *pedo_data) {
  uint8_t apex_data[4] = {0U};
  uint8_t int_status3 = 0U;

  if (pedo_data == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_APEX_DATA0, apex_data, 4U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS3, &int_status3, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  pedo_data->step_count =
      (uint16_t)(((uint16_t)apex_data[1] << 8U) | apex_data[0]);
  pedo_data->cadence_raw = apex_data[2];
  pedo_data->activity = (ICM42670_ApexActivity_t)(apex_data[3] & 0x03U);
  pedo_data->step_detected =
      ((int_status3 & ICM42670_INT_STATUS3_STEP_DET) != 0U) ? 1U : 0U;
  pedo_data->overflow =
      ((int_status3 & ICM42670_INT_STATUS3_STEP_OVF) != 0U) ? 1U : 0U;

  return ICM42670_OK;
}

ICM42670_Status_t
ICM42670_Enable_Tilt(const ICM42670_Config *config,
                     const ICM42670_TiltConfig_t *tilt_config) {
  ICM42670_Status_t status;
  uint8_t wait_sel;

  if (tilt_config == NULL) {
    return ICM42670_ERROR;
  }

  wait_sel = SelectNearestIndex(
      ICM42670_TiltWaitSeconds, (uint8_t)(sizeof(ICM42670_TiltWaitSeconds) /
                                          sizeof(ICM42670_TiltWaitSeconds[0])),
      tilt_config->wait_time_s);

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG5,
      ICM42670_APEX_CONFIG5_TILT_WAIT_MASK, (uint8_t)(wait_sel << 6U));
  if (status != ICM42670_OK) {
    return status;
  }

  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_TILT_ENABLE,
                               ICM42670_APEX_ENABLE);
}

ICM42670_Status_t ICM42670_Disable_Tilt(const ICM42670_Config *config) {
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_TILT_ENABLE,
                               ICM42670_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_Read_Tilt(const ICM42670_Config *config,
                                     uint8_t *tilt_detected) {
  uint8_t int_status3 = 0U;

  if (tilt_detected == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS3, &int_status3, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *tilt_detected =
      ((int_status3 & ICM42670_INT_STATUS3_TILT) != 0U) ? 1U : 0U;
  return ICM42670_OK;
}

ICM42670_Status_t
ICM42670_Enable_Low_G(const ICM42670_Config *config,
                      const ICM42670_LowGConfig_t *low_g_config) {
  ICM42670_Status_t status;
  uint8_t threshold_sel;
  uint8_t sample_sel;
  uint8_t field_value;

  if (low_g_config == NULL) {
    return ICM42670_ERROR;
  }

  threshold_sel = SelectNearestIndex(
      ICM42670_LowGThresholdMg, (uint8_t)(sizeof(ICM42670_LowGThresholdMg) /
                                          sizeof(ICM42670_LowGThresholdMg[0])),
      low_g_config->threshold_mg);
  sample_sel = (uint8_t)(ClampU8(low_g_config->sample_count, 1U, 8U) - 1U);
  field_value = (uint8_t)((threshold_sel << 3U) | sample_sel);

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG10,
      ICM42670_APEX_CONFIG10_LOWG_MASK, field_value);
  if (status != ICM42670_OK) {
    return status;
  }

  /* Low-g is reported by the freefall/low-g APEX engine. */
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_FF_ENABLE,
                               ICM42670_APEX_ENABLE);
}

ICM42670_Status_t ICM42670_Disable_Low_G(const ICM42670_Config *config) {
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_FF_ENABLE,
                               ICM42670_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_Read_Low_G(const ICM42670_Config *config,
                                      uint8_t *low_g_detected) {
  uint8_t int_status3 = 0U;

  if (low_g_detected == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS3, &int_status3, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *low_g_detected =
      ((int_status3 & ICM42670_INT_STATUS3_LOWG) != 0U) ? 1U : 0U;
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_Enable_Free_Fall(
    const ICM42670_Config *config,
    const ICM42670_FreeFallConfig_t *free_fall_config) {
  ICM42670_Status_t status;
  uint8_t min_sel;
  uint8_t max_sel;
  uint8_t debounce_sel;
  uint8_t duration_value;

  if (free_fall_config == NULL) {
    return ICM42670_ERROR;
  }

  min_sel = SelectNearestIndex(
      ICM42670_FreeFallMinDistanceCm,
      (uint8_t)(sizeof(ICM42670_FreeFallMinDistanceCm) /
                sizeof(ICM42670_FreeFallMinDistanceCm[0])),
      free_fall_config->min_distance_cm);
  max_sel = SelectNearestIndex(
      ICM42670_FreeFallMaxDistanceCm,
      (uint8_t)(sizeof(ICM42670_FreeFallMaxDistanceCm) /
                sizeof(ICM42670_FreeFallMaxDistanceCm[0])),
      free_fall_config->max_distance_cm);
  debounce_sel = SelectNearestIndex(
      ICM42670_FreeFallDebounceMs,
      (uint8_t)(sizeof(ICM42670_FreeFallDebounceMs) /
                sizeof(ICM42670_FreeFallDebounceMs[0])),
      free_fall_config->debounce_ms);
  duration_value = (uint8_t)((max_sel << 4U) | min_sel);

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG12,
      ICM42670_APEX_CONFIG12_FF_DURATION_MASK, duration_value);
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG9,
      ICM42670_APEX_CONFIG9_FF_DEBOUNCE_MASK,
      (uint8_t)(debounce_sel << 4U));
  if (status != ICM42670_OK) {
    return status;
  }

  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_FF_ENABLE,
                               ICM42670_APEX_ENABLE);
}

ICM42670_Status_t ICM42670_Disable_Free_Fall(const ICM42670_Config *config) {
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_FF_ENABLE,
                               ICM42670_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_Read_Free_Fall(
    const ICM42670_Config *config, ICM42670_FreeFallData_t *free_fall_data) {
  uint8_t apex_data[2] = {0U};
  uint8_t int_status3 = 0U;

  if (free_fall_data == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_APEX_DATA4, apex_data, 2U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS3, &int_status3, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  free_fall_data->detected =
      ((int_status3 & ICM42670_INT_STATUS3_FF) != 0U) ? 1U : 0U;
  free_fall_data->duration_samples =
      (uint16_t)(((uint16_t)apex_data[1] << 8U) | apex_data[0]);

  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_Enable_Wake_On_Motion(
    const ICM42670_Config *config,
    const ICM42670_WakeOnMotionConfig_t *wake_on_motion_config) {
  ICM42670_Status_t status;

  if (wake_on_motion_config == NULL) {
    return ICM42670_ERROR;
  }

  status = ICM42670_WriteMreg1(
      config, ICM42670_MREG1_ACCEL_WOM_X_THR,
      ConvertWomThresholdMg(wake_on_motion_config->x_threshold_mg));
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_WriteMreg1(
      config, ICM42670_MREG1_ACCEL_WOM_Y_THR,
      ConvertWomThresholdMg(wake_on_motion_config->y_threshold_mg));
  if (status != ICM42670_OK) {
    return status;
  }

  status = ICM42670_WriteMreg1(
      config, ICM42670_MREG1_ACCEL_WOM_Z_THR,
      ConvertWomThresholdMg(wake_on_motion_config->z_threshold_mg));
  if (status != ICM42670_OK) {
    return status;
  }

  return SetWomFeature(config, ICM42670_APEX_ENABLE);
}

ICM42670_Status_t
ICM42670_Disable_Wake_On_Motion(const ICM42670_Config *config) {
  return SetWomFeature(config, ICM42670_APEX_DISABLE);
}

ICM42670_Status_t ICM42670_Read_Wake_On_Motion(
    const ICM42670_Config *config,
    ICM42670_WakeOnMotionData_t *wake_on_motion_data) {
  uint8_t int_status2 = 0U;

  if (wake_on_motion_data == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS2, &int_status2, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  wake_on_motion_data->x_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_X) != 0U) ? 1U : 0U;
  wake_on_motion_data->y_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_Y) != 0U) ? 1U : 0U;
  wake_on_motion_data->z_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_Z) != 0U) ? 1U : 0U;

  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_Enable_Significant_Motion(
    const ICM42670_Config *config,
    const ICM42670_SignificantMotionConfig_t *significant_motion_config) {
  ICM42670_Status_t status;
  uint8_t sensitivity;

  if (significant_motion_config == NULL) {
    return ICM42670_ERROR;
  }

  sensitivity =
      ClampU8(significant_motion_config->sensitivity_level, 0U, 4U);
  status = ICM42670_UpdateMreg1Bits(
      config, ICM42670_MREG1_APEX_CONFIG9,
      ICM42670_APEX_CONFIG9_SMD_SENSITIVITY_MASK,
      (uint8_t)(sensitivity << 1U));
  if (status != ICM42670_OK) {
    return status;
  }

  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_SMD_ENABLE,
                               ICM42670_APEX_ENABLE);
}

ICM42670_Status_t
ICM42670_Disable_Significant_Motion(const ICM42670_Config *config) {
  return SetApexConfig1Feature(config, ICM42670_APEX_CONFIG1_SMD_ENABLE,
                               ICM42670_APEX_DISABLE);
}

ICM42670_Status_t
ICM42670_Read_Significant_Motion(const ICM42670_Config *config,
                                 uint8_t *significant_motion_detected) {
  uint8_t int_status2 = 0U;

  if (significant_motion_detected == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS2, &int_status2, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  *significant_motion_detected =
      ((int_status2 & ICM42670_INT_STATUS2_SMD) != 0U) ? 1U : 0U;
  return ICM42670_OK;
}

ICM42670_Status_t ICM42670_Read_Apex(const ICM42670_Config *config,
                                     ICM42670_ApexData_t *apex_data) {
  uint8_t pedo_raw[4] = {0U};
  uint8_t free_fall_raw[2] = {0U};
  uint8_t int_status2 = 0U;
  uint8_t int_status3 = 0U;

  if (apex_data == NULL) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_APEX_DATA0, pedo_raw, 4U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_APEX_DATA4, free_fall_raw, 2U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS2, &int_status2, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  if (ICM42670_ReadReg(config, ICM42670_REG_INT_STATUS3, &int_status3, 1U) !=
      ICM42670_OK) {
    return ICM42670_ERROR;
  }

  apex_data->pedo.step_count =
      (uint16_t)(((uint16_t)pedo_raw[1] << 8U) | pedo_raw[0]);
  apex_data->pedo.cadence_raw = pedo_raw[2];
  apex_data->pedo.activity = (ICM42670_ApexActivity_t)(pedo_raw[3] & 0x03U);
  apex_data->pedo.step_detected =
      ((int_status3 & ICM42670_INT_STATUS3_STEP_DET) != 0U) ? 1U : 0U;
  apex_data->pedo.overflow =
      ((int_status3 & ICM42670_INT_STATUS3_STEP_OVF) != 0U) ? 1U : 0U;

  apex_data->free_fall.detected =
      ((int_status3 & ICM42670_INT_STATUS3_FF) != 0U) ? 1U : 0U;
  apex_data->free_fall.duration_samples =
      (uint16_t)(((uint16_t)free_fall_raw[1] << 8U) | free_fall_raw[0]);

  apex_data->wake_on_motion.x_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_X) != 0U) ? 1U : 0U;
  apex_data->wake_on_motion.y_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_Y) != 0U) ? 1U : 0U;
  apex_data->wake_on_motion.z_detected =
      ((int_status2 & ICM42670_INT_STATUS2_WOM_Z) != 0U) ? 1U : 0U;

  apex_data->tilt_detected =
      ((int_status3 & ICM42670_INT_STATUS3_TILT) != 0U) ? 1U : 0U;
  apex_data->low_g_detected =
      ((int_status3 & ICM42670_INT_STATUS3_LOWG) != 0U) ? 1U : 0U;
  apex_data->significant_motion_detected =
      ((int_status2 & ICM42670_INT_STATUS2_SMD) != 0U) ? 1U : 0U;

  return ICM42670_OK;
}
