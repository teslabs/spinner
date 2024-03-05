/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_currsmp_shunt

#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stm32_ll_adc.h>

#include <spinner/drivers/currsmp.h>
#include <spinner/utils/stm32_adc.h>

LOG_MODULE_REGISTER(currsmp_shunt_stm32, CONFIG_SPINNER_CURRSMP_LOG_LEVEL);

/*******************************************************************************
 * Private
 ******************************************************************************/

struct currsmp_shunt_stm32_config {
	ADC_TypeDef *adc;
	struct stm32_pclken pclken;
	uint32_t adc_irq;
	uint8_t adc_resolution;
	uint16_t adc_tsample;
	uint32_t adc_ch_a;
	uint32_t adc_ch_b;
	uint32_t adc_ch_c;
	uint32_t adc_trigger;
	const struct pinctrl_dev_config *pcfg;
};

struct currsmp_shunt_stm32_data {
	currsmp_regulation_cb_t regulation_cb;
	void *regulation_ctx;
	uint16_t i_a_offset;
	uint16_t i_b_offset;
	uint16_t i_c_offset;
	uint8_t sector;
	uint32_t jsqr[3];
};

ISR_DIRECT_DECLARE(adc_irq)
{
	const struct device *dev = DEVICE_DT_INST_GET(0);
	const struct currsmp_shunt_stm32_config *config = dev->config;
	struct currsmp_shunt_stm32_data *data = dev->data;

	if (LL_ADC_IsActiveFlag_JEOS(config->adc)) {
		LL_ADC_ClearFlag_JEOS(config->adc);
		data->regulation_cb(data->regulation_ctx);
	}

	return 0;
}

/**
 * Compute the ADC injected sequence register (JSQR) for the given 2 channels.
 *
 * @param[in] trigger ADC trigger.
 * @param[in] rank1_ch Rank 1 channel.
 * @param[in] rank2_ch Rank 2 channel.
 *
 * @return Computed JSQR register value.
 */
static uint32_t adc_calc_jsqr(uint32_t trigger, uint32_t rank1_ch,
			      uint32_t rank2_ch)
{
	uint32_t jsqr;

	uint8_t ch1 = __LL_ADC_CHANNEL_TO_DECIMAL_NB(rank1_ch);
	uint8_t ch2 = __LL_ADC_CHANNEL_TO_DECIMAL_NB(rank2_ch);

#ifdef CONFIG_SOC_SERIES_STM32F3X
	/* F3X ADC uses channels 1..18, indexed from 0..17 */
	ch1--;
	ch2--;
#endif

	jsqr = ((ch1 & ADC_INJ_RANK_ID_JSQR_MASK)
		<< ADC_INJ_RANK_1_JSQR_BITOFFSET_POS) |
	       ((ch2 & ADC_INJ_RANK_ID_JSQR_MASK)
		<< ADC_INJ_RANK_2_JSQR_BITOFFSET_POS) |
	       LL_ADC_INJ_TRIG_EXT_RISING | trigger | 1U;

	return jsqr;
}

/**
 * @brief Configure ADC.
 *
 * @param[in] dev Current sampling device.
 *
 * @return 0 on success, negative errno otherwise.
 */
static int adc_configure(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	int ret;
	const struct device *clk;
	uint32_t smp;
	LL_ADC_CommonInitTypeDef adc_cinit;
	LL_ADC_InitTypeDef adc_init;
	LL_ADC_REG_InitTypeDef adc_rinit;
	LL_ADC_INJ_InitTypeDef adc_jinit;
	uint32_t adc_clk;

	/* enable ADC clock */
	clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);
	ret = clock_control_on(clk, (clock_control_subsys_t *)&config->pclken);
	if (ret < 0) {
		LOG_ERR("Could not turn on ADC clock (%d)", ret);
		return ret;
	}

	/* configure common ADC instance */
	LL_ADC_CommonStructInit(&adc_cinit);
	if (config->adc_resolution == 6U) {
		adc_cinit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;
	} else {
		adc_cinit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
	}
	if (LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(config->adc),
			      &adc_cinit) != SUCCESS) {
		LOG_ERR("Could not initialize common ADC");
		return -EIO;
	}

	/* configure ADC */
	LL_ADC_StructInit(&adc_init);

	ret = stm32_adc_res_get(config->adc_resolution, &adc_init.Resolution);
	if (ret < 0) {
		LOG_ERR("Unsupported ADC resolution");
		return ret;
	}

	if (LL_ADC_Init(config->adc, &adc_init) != SUCCESS) {
		LOG_ERR("Could not initialize ADC");
		return -EIO;
	}

	/* configure ADC (regular) */
	LL_ADC_REG_StructInit(&adc_rinit);
	adc_rinit.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
	if (LL_ADC_REG_Init(config->adc, &adc_rinit) != SUCCESS) {
		LOG_ERR("Could not initialize ADC regular group");
		return -EIO;
	}

	/* configure ADC (injected) */
	LL_ADC_INJ_StructInit(&adc_jinit);
	adc_jinit.TriggerSource =
		config->adc_trigger | LL_ADC_INJ_TRIG_EXT_RISING;
	if (LL_ADC_INJ_Init(config->adc, &adc_jinit) != SUCCESS) {
		LOG_ERR("Could not initialize ADC injected group");
		return -EIO;
	}

	/* configure sampling time */
	ret = stm32_adc_smp_get(config->adc_tsample, &smp);
	if (ret < 0) {
		LOG_ERR("Unsupported ADC sampling time");
		return ret;
	}

	LL_ADC_SetChannelSamplingTime(config->adc, config->adc_ch_a, smp);
	LL_ADC_SetChannelSamplingTime(config->adc, config->adc_ch_b, smp);
	LL_ADC_SetChannelSamplingTime(config->adc, config->adc_ch_c, smp);

	/* enable internal ADC regulator */
#if defined(CONFIG_SOC_SERIES_STM32G4X)
	LL_ADC_DisableDeepPowerDown(config->adc);
#endif
	LL_ADC_EnableInternalRegulator(config->adc);
	k_busy_wait(LL_ADC_DELAY_INTERNAL_REGUL_STAB_US);
	if (!LL_ADC_IsInternalRegulatorEnabled(config->adc)) {
		LOG_ERR("ADC internal regulator not enabled within expected "
			"time");
		return -EIO;
	}

	/* calibrate ADC */
	LL_ADC_StartCalibration(config->adc, LL_ADC_SINGLE_ENDED);
	while (LL_ADC_IsCalibrationOnGoing(config->adc))
		;

	/* wait to enable ADC after calibration */
	ret = stm32_adc_clk_get(config->adc, &config->pclken, &adc_clk);
	if (ret < 0) {
		return ret;
	}

	k_busy_wait(MAX(1U, (uint32_t)((1.0e6f / (float)adc_clk) *
				       LL_ADC_DELAY_CALIB_ENABLE_ADC_CYCLES)));

	/* enable ADC */
	LL_ADC_Enable(config->adc);
	while (LL_ADC_IsActiveFlag_ADRDY(config->adc) != 1U)
		;

	/* configure ADC IRQ */
	LL_ADC_EnableIT_JEOS(config->adc);

	IRQ_DIRECT_CONNECT(DT_IRQ_BY_IDX(DT_INST_PARENT(0), 0, irq),
			   DT_IRQ_BY_IDX(DT_INST_PARENT(0), 0, priority),
			   adc_irq, IRQ_ZERO_LATENCY);
	irq_enable(config->adc_irq);

	return 0;
}

/**
 * @brief Perform regular ADC read.
 *
 * @param dev Current sampling device
 * @param channel ADC channel
 *
 * @return Sample value.
 */
static uint16_t adc_read(const struct device *dev, uint32_t channel)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	/* configure sequencer: only one channel */
	LL_ADC_REG_SetSequencerLength(config->adc, LL_ADC_REG_SEQ_SCAN_DISABLE);
	LL_ADC_REG_SetSequencerRanks(config->adc, LL_ADC_REG_RANK_1, channel);

	/* perform regular conversion */
	LL_ADC_REG_StartConversion(config->adc);
	while (LL_ADC_IsActiveFlag_EOS(config->adc) != 1U)
		;

	LL_ADC_ClearFlag_EOS(config->adc);

	return (uint16_t)LL_ADC_REG_ReadConversionData32(config->adc);
}

/*******************************************************************************
 * API
 ******************************************************************************/

static void currsmp_shunt_stm32_configure(const struct device *dev,
					  currsmp_regulation_cb_t regulation_cb,
					  void *ctx)
{
	struct currsmp_shunt_stm32_data *data = dev->data;

	data->regulation_cb = regulation_cb;
	data->regulation_ctx = ctx;
}

static void currsmp_shunt_stm32_get_currents(const struct device *dev,
					     struct currsmp_curr *curr)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;
	struct currsmp_shunt_stm32_data *data = dev->data;

	uint16_t val_ch1;
	uint16_t val_ch2;
	int16_t i_a = 0, i_b = 0, i_c = 0;

	val_ch1 = (uint16_t)LL_ADC_INJ_ReadConversionData32(config->adc,
							    LL_ADC_INJ_RANK_1);
	val_ch2 = (uint16_t)LL_ADC_INJ_ReadConversionData32(config->adc,
							    LL_ADC_INJ_RANK_2);

	switch (data->sector) {
	case 1U:
		i_b = data->i_b_offset - val_ch1;
		i_c = data->i_c_offset - val_ch2;
		i_a = -(i_b + i_c);
		break;
	case 2U:
		i_a = data->i_a_offset - val_ch1;
		i_c = data->i_c_offset - val_ch2;
		i_b = -(i_a + i_c);
		break;
	case 3U:
		i_a = data->i_a_offset - val_ch1;
		i_c = data->i_c_offset - val_ch2;
		i_b = -(i_a + i_c);
		break;
	case 4U:
		i_a = data->i_a_offset - val_ch2;
		i_b = data->i_b_offset - val_ch1;
		i_c = -(i_a + i_b);
		break;
	case 5U:
		i_a = data->i_a_offset - val_ch2;
		i_b = data->i_b_offset - val_ch1;
		i_c = -(i_a + i_b);
		break;
	case 6U:
		i_b = data->i_b_offset - val_ch1;
		i_c = data->i_c_offset - val_ch2;
		i_a = -(i_b + i_c);
		break;
	default:
		__ASSERT(NULL, "Unexpected sector");
		break;
	}

	curr->i_a = (float)i_a / (2U << (config->adc_resolution - 1U));
	curr->i_b = (float)i_b / (2U << (config->adc_resolution - 1U));
	curr->i_c = (float)i_c / (2U << (config->adc_resolution - 1U));
}

static void currsmp_shunt_stm32_set_sector(const struct device *dev,
					   uint8_t sector)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;
	struct currsmp_shunt_stm32_data *data = dev->data;

	data->sector = sector;
	config->adc->JSQR = data->jsqr[sector / 2U % 3U];
}

static uint32_t currsmp_shunt_stm32_get_smp_time(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	int ret;
	uint32_t clk;
	float t_sar;

	ret = stm32_adc_clk_get(config->adc, &config->pclken, &clk);
	if (ret < 0) {
		LOG_ERR("Could not obtain ADC clock rate");
		return 0U;
	}

	ret = stm32_adc_t_sar_get(config->adc_resolution, &t_sar);
	if (ret < 0) {
		LOG_ERR("Could not obtain ADC SAR time");
		return 0U;
	}

	return (uint32_t)((1.0e9f / (float)clk) *
			  (t_sar + 2.0f * (float)config->adc_tsample));
}

static void currsmp_shunt_stm32_start(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;
	struct currsmp_shunt_stm32_data *data = dev->data;

	/* calibrate a, b, c offset */
	LL_ADC_ClearFlag_EOS(config->adc);

	data->i_a_offset = adc_read(dev, config->adc_ch_a);
	data->i_b_offset = adc_read(dev, config->adc_ch_b);
	data->i_c_offset = adc_read(dev, config->adc_ch_c);

	/* start injected conversions (triggered by sv-pwm) */
	LL_ADC_ClearFlag_JEOS(config->adc);
	LL_ADC_INJ_StartConversion(config->adc);
}

static void currsmp_shunt_stm32_stop(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	LL_ADC_INJ_StopConversion(config->adc);
	while (LL_ADC_INJ_IsStopConversionOngoing(config->adc) != 0U)
		;

	while (LL_ADC_INJ_IsConversionOngoing(config->adc) != 0U)
		;
}

static void currsmp_shunt_stm32_pause(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	LL_ADC_DisableIT_JEOS(config->adc);
}

static void currsmp_shunt_stm32_resume(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;

	LL_ADC_EnableIT_JEOS(config->adc);
}

static const struct currsmp_driver_api currsmp_shunt_stm32_driver_api = {
	.configure = currsmp_shunt_stm32_configure,
	.get_currents = currsmp_shunt_stm32_get_currents,
	.set_sector = currsmp_shunt_stm32_set_sector,
	.get_smp_time = currsmp_shunt_stm32_get_smp_time,
	.start = currsmp_shunt_stm32_start,
	.stop = currsmp_shunt_stm32_stop,
	.pause = currsmp_shunt_stm32_pause,
	.resume = currsmp_shunt_stm32_resume,
};

/*******************************************************************************
 * Initialization
 ******************************************************************************/

static int currsmp_shunt_stm32_init(const struct device *dev)
{
	const struct currsmp_shunt_stm32_config *config = dev->config;
	struct currsmp_shunt_stm32_data *data = dev->data;

	int ret;

	/* configure pinmux */
	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		LOG_ERR("pinctrl setup failed (%d)", ret);
		return ret;
	}

	/* configure ADC */
	ret = adc_configure(dev);
	if (ret < 0) {
		return ret;
	}

	/* pre-compute ADC injected sequences */
	data->jsqr[0] = adc_calc_jsqr(config->adc_trigger, config->adc_ch_b,
				      config->adc_ch_c);
	data->jsqr[1] = adc_calc_jsqr(config->adc_trigger, config->adc_ch_a,
				      config->adc_ch_c);
	data->jsqr[2] = adc_calc_jsqr(config->adc_trigger, config->adc_ch_b,
				      config->adc_ch_a);

	return 0;
}

PINCTRL_DT_INST_DEFINE(0);

static const struct currsmp_shunt_stm32_config currsmp_shunt_stm32_config = {
	.adc = (ADC_TypeDef *)DT_REG_ADDR(DT_INST_PARENT(0)),
	.pclken = STM32_CLOCK_INFO(0, DT_INST_PARENT(0)),
	.adc_irq = DT_IRQ_BY_IDX(DT_INST_PARENT(0), 0, irq),
	.adc_resolution = DT_INST_PROP(0, adc_resolution),
	.adc_tsample = DT_INST_PROP(0, adc_tsample),
	.adc_ch_a = __LL_ADC_DECIMAL_NB_TO_CHANNEL(
		DT_INST_PROP_BY_IDX(0, adc_channels, 0)),
	.adc_ch_b = __LL_ADC_DECIMAL_NB_TO_CHANNEL(
		DT_INST_PROP_BY_IDX(0, adc_channels, 1)),
	.adc_ch_c = __LL_ADC_DECIMAL_NB_TO_CHANNEL(
		DT_INST_PROP_BY_IDX(0, adc_channels, 2)),
	.adc_trigger = DT_INST_PROP(0, adc_trigger),
	.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(0),
};

static struct currsmp_shunt_stm32_data currsmp_shunt_stm32_data;

DEVICE_DT_INST_DEFINE(0, &currsmp_shunt_stm32_init, NULL,
		      &currsmp_shunt_stm32_data, &currsmp_shunt_stm32_config,
		      POST_KERNEL, CONFIG_SPINNER_CURRSMP_INIT_PRIORITY,
		      &currsmp_shunt_stm32_driver_api);
