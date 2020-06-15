/**
 * @file
 *
 * STM32 ADC Utilities.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_LIB_UTILS_STM32_ADC_H_
#define _SPINNER_LIB_UTILS_STM32_ADC_H_

#include <drivers/clock_control/stm32_clock_control.h>

#include <stm32_ll_adc.h>

/**
 * @defgroup spinner_lib_utils Utility APIs
 * @{
 * @}
 */

/**
 * @defgroup spinner_utils_stm32_adc STM32 ADC Utilities
 * @ingroup spinner_lib_utils
 * @{
 */

/**
 * @brief Obtain the RES fields in the ADC CFGR register given the sampling
 * resolution in bits.
 *
 * @param[in] res_bits Sampling resolution in bits.
 * @param[out] res Obtained RES register value.
 * @return int 0 on success, -ENOTSUP if fiven bit resolution is not supported.
 */
int stm32_adc_res_get(uint8_t res_bits, uint32_t *res);

/**
 * @brief Obtain the value of the SMP fields in the ADC SMPR register given
 * sampling time in cycles.
 *
 * @note For decimal sampling times, @p smp_time must be rounded up, e.g. 19.5
 * cycles should be provided as 20.
 *
 * @param[in] smp_time ADC sampling time in cycles
 * @param[out] smp Obtained SMP register value.
 * @return int 0 on success, -ENOTSUP if given sampling time is not supported.
 */
int stm32_adc_smp_get(uint32_t smp_time, uint32_t *smp);

/**
 * Obtain ADC clock rate.
 *
 * @param[in] adc ADC instance.
 * @param[in] pclken ADC clock control subsystem.
 * @param[out] clk Where ADC clock (in Hz) will be stored.
 *
 * @return 0 on success, error code otherwise.
 */
int stm32_adc_clk_get(ADC_TypeDef *adc, const struct stm32_pclken *pclken,
		      uint32_t *clk);

/**
 * Obtain ADC clock SAR time.
 *
 * @param[in] res_bits Sampling resolution in bits.
 * @param[out] t_sar Where computed SAR time (in cycles) time will be stored.
 *
 * @return 0 on success, error code otherwise.
 */
int stm32_adc_t_sar_get(uint8_t res_bits, float *t_sar);

/** @} */

#endif /* _SPINNER_LIB_UTILS_STM32_ADC_H_ */
