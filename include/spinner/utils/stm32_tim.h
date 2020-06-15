/**
 * @file
 *
 * STM32 Timer Utilities.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_LIB_UTILS_STM32_TIM_H_
#define _SPINNER_LIB_UTILS_STM32_TIM_H_

#include <drivers/clock_control/stm32_clock_control.h>

/**
 * @defgroup spinner_utils_stm32_tim STM32 Timer Utilities
 * @ingroup spinner_lib_utils
 * @{
 */

/**
 * Obtain timer clock speed.
 *
 * @param[in] pclken Timer clock control subsystem.
 * @param[out] tim_clk Where computed timer clock will be stored.
 *
 * @return 0 on success, error code otherwise.
 */
int stm32_tim_clk_get(const struct stm32_pclken *pclken, uint32_t *tim_clk);

/** @} */

#endif /* _SPINNER_LIB_UTILS_STM32_TIM_H_ */
