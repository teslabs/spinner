/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>

#include <spinner/utils/stm32_adc.h>

int stm32_adc_res_get(uint8_t res_bits, uint32_t *res)
{
	switch (res_bits) {
#if defined(CONFIG_SOC_SERIES_STM32F3X) || defined(CONFIG_SOC_SERIES_STM32G4X)
	case 6U:
		*res = LL_ADC_RESOLUTION_6B;
		break;
	case 8U:
		*res = LL_ADC_RESOLUTION_8B;
		break;
	case 10U:
		*res = LL_ADC_RESOLUTION_10B;
		break;
	case 12U:
		*res = LL_ADC_RESOLUTION_12B;
		break;
#endif
	default:
		return -ENOTSUP;
	}

	return 0;
}

int stm32_adc_smp_get(uint32_t smp_time, uint32_t *smp)
{
	switch(smp_time) {
#if defined(CONFIG_SOC_SERIES_STM32F3X)
	case 2U:
		*smp = LL_ADC_SAMPLINGTIME_1CYCLE_5;
		break;
	case 3U:
		*smp = LL_ADC_SAMPLINGTIME_2CYCLES_5;
		break;
	case 5U:
		*smp = LL_ADC_SAMPLINGTIME_4CYCLES_5;
		break;
	case 8U:
		*smp = LL_ADC_SAMPLINGTIME_7CYCLES_5;
		break;
	case 20U:
		*smp = LL_ADC_SAMPLINGTIME_19CYCLES_5;
		break;
	case 62U:
		*smp = LL_ADC_SAMPLINGTIME_61CYCLES_5;
		break;
	case 181U:
		*smp = LL_ADC_SAMPLINGTIME_181CYCLES_5;
		break;
	case 602U:
		*smp = LL_ADC_SAMPLINGTIME_601CYCLES_5;
		break;
#elif defined(CONFIG_SOC_SERIES_STM32G4X)
	case 3U:
		*smp = LL_ADC_SAMPLINGTIME_2CYCLES_5;
		break;
	case 7U:
		*smp = LL_ADC_SAMPLINGTIME_6CYCLES_5;
		break;
	case 13U:
		*smp = LL_ADC_SAMPLINGTIME_12CYCLES_5;
		break;
	case 25U:
		*smp = LL_ADC_SAMPLINGTIME_24CYCLES_5;
		break;
	case 48U:
		*smp = LL_ADC_SAMPLINGTIME_47CYCLES_5;
		break;
	case 93U:
		*smp = LL_ADC_SAMPLINGTIME_92CYCLES_5;
		break;
	case 248U:
		*smp = LL_ADC_SAMPLINGTIME_247CYCLES_5;
		break;
	case 641U:
		*smp = LL_ADC_SAMPLINGTIME_640CYCLES_5;
		break;
#endif
	default:
		return -ENOTSUP;
	}

	return 0;
}

int stm32_adc_clk_get(ADC_TypeDef *adc, const struct stm32_pclken *pclken,
		      uint32_t *clk)
{
	const struct device *clk_dev;
	int ret;
	uint32_t div;

	/* obtain ADC clock rate */
	clk_dev = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);
	ret = clock_control_get_rate(clk_dev, (clock_control_subsys_t *)pclken,
				     clk);
	if (ret < 0) {
		return ret;
	}

	/* obtain divisor */
	div = LL_ADC_GetCommonClock(__LL_ADC_COMMON_INSTANCE(adc));
	switch (div) {
	case LL_ADC_CLOCK_SYNC_PCLK_DIV1:
		break;
	case LL_ADC_CLOCK_SYNC_PCLK_DIV2:
		*clk = *clk >> 1U;
		break;
	case LL_ADC_CLOCK_SYNC_PCLK_DIV4:
		*clk = *clk >> 2U;
		break;
	default:
		return -ENOTSUP;
	}

	return 0;
}

int stm32_adc_t_sar_get(uint8_t res_bits, float *t_sar)
{
	/* obtain t_sar (in clock cycles) */
	switch (res_bits) {
#if defined(CONFIG_SOC_SERIES_STM32F3X) || defined(CONFIG_SOC_SERIES_STM32G4X)
	case 6U:
		*t_sar = 6.5f;
		break;
	case 8U:
		*t_sar = 8.5f;
		break;
	case 10U:
		*t_sar = 10.5f;
		break;
	case 12U:
		*t_sar = 12.5f;
		break;
#endif
	default:
		return -ENOTSUP;
	}

	return 0;
}
