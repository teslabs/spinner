/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>

#include <spinner/utils/stm32_tim.h>

int stm32_tim_clk_get(const struct stm32_pclken *pclken, uint32_t *tim_clk)
{
	int ret;
	const struct device *clk;
	uint32_t bus_clk, apb_psc;

	clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

	ret = clock_control_get_rate(clk, (clock_control_subsys_t *)pclken,
				     &bus_clk);
	if (ret < 0) {
		return ret;
	}

	if (pclken->bus == STM32_CLOCK_BUS_APB1) {
		apb_psc = STM32_APB1_PRESCALER;
	}
	else {
		apb_psc = STM32_APB2_PRESCALER;
	}

	/*
	 * If the APB prescaler equals 1, the timer clock frequencies
	 * are set to the same frequency as that of the APB domain.
	 * Otherwise, they are set to twice (×2) the frequency of the
	 * APB domain.
	 */
	if (apb_psc == 1U) {
		*tim_clk = bus_clk;
	} else {
		*tim_clk = bus_clk * 2U;
	}

	return 0;
}
