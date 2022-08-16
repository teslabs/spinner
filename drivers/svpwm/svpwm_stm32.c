/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_svpwm

#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/logging/log.h>
#include <soc.h>

#include <stm32_ll_tim.h>

#include <spinner/drivers/currsmp.h>
#include <spinner/drivers/svpwm.h>
#include <spinner/svm/svm.h>
#include <spinner/utils/stm32_tim.h>

LOG_MODULE_REGISTER(svpwm_stm32, CONFIG_SPINNER_SVPWM_LOG_LEVEL);

/*******************************************************************************
 * Private
 ******************************************************************************/

struct svpwm_stm32_config {
	TIM_TypeDef *timer;
	struct stm32_pclken pclken;
	bool enable_comp_outputs;
	uint32_t t_dead;
	uint32_t t_rise;
	const struct device *currsmp;
	const struct gpio_dt_spec *enable;
	size_t enable_len;
	const struct pinctrl_dev_config *pcfg;
};

struct svpwm_stm32_data {
	uint32_t period;
	svm_t svm;
};

/*******************************************************************************
 * API
 ******************************************************************************/

static void svpwm_stm32_start(const struct device *dev)
{
	const struct svpwm_stm32_config *config = dev->config;
	struct svpwm_stm32_data *data = dev->data;

	svm_init(&data->svm);
	data->svm.sector = 5U;
	currsmp_set_sector(config->currsmp, data->svm.sector);

	/* activate enable pins if available */
	for (size_t i = 0U; i < config->enable_len; i++) {
		gpio_pin_set(config->enable[i].port, config->enable[i].pin, 1);
	}

	/* configure timer OC for a, b, c */
	LL_TIM_OC_SetCompareCH1(config->timer, data->period / 2U);
	LL_TIM_OC_SetCompareCH2(config->timer, data->period / 2U);
	LL_TIM_OC_SetCompareCH3(config->timer, data->period / 2U);

	LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH3);
	if (config->enable_comp_outputs) {
		LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH1N);
		LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH2N);
		LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH3N);
	}

	/* configure timer OC for ADC trigger */
	LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH4);

	/* start timer */
	LL_TIM_EnableAllOutputs(config->timer);

	LL_TIM_EnableCounter(config->timer);
}

static void svpwm_stm32_stop(const struct device *dev)
{
	const struct svpwm_stm32_config *config = dev->config;

	/* stop timer */
	LL_TIM_DisableCounter(config->timer);

	LL_TIM_DisableAllOutputs(config->timer);

	LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH3);
	if (config->enable_comp_outputs) {
		LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH1N);
		LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH2N);
		LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH3N);
	}

	LL_TIM_CC_DisableChannel(config->timer, LL_TIM_CHANNEL_CH4);

	/* deactivate enable pins if available */
	for (size_t i = 0U; i < config->enable_len; i++) {
		gpio_pin_set(config->enable[i].port, config->enable[i].pin, 0);
	}
}

static void svpwm_stm32_set_phase_voltages(const struct device *dev,
					   float v_alpha, float v_beta)
{
	const struct svpwm_stm32_config *config = dev->config;
	struct svpwm_stm32_data *data = dev->data;

	const svm_duties_t *duties = &data->svm.duties;

	/* space-vector modulation */
	svm_set(&data->svm, v_alpha, v_beta);

	/* program duties */
	LL_TIM_OC_SetCompareCH1(config->timer,
				(uint32_t)(data->period * duties->a));
	LL_TIM_OC_SetCompareCH2(config->timer,
				(uint32_t)(data->period * duties->b));
	LL_TIM_OC_SetCompareCH3(config->timer,
				(uint32_t)(data->period * duties->c));

	/* inform current sampling device about current sector */
	currsmp_set_sector(config->currsmp, data->svm.sector);
}

static const struct svpwm_driver_api svpwm_stm32_driver_api = {
	.start = svpwm_stm32_start,
	.stop = svpwm_stm32_stop,
	.set_phase_voltages = svpwm_stm32_set_phase_voltages,
};

/*******************************************************************************
 * Initialization
 ******************************************************************************/

static int svpwm_stm32_init(const struct device *dev)
{
	const struct svpwm_stm32_config *config = dev->config;
	struct svpwm_stm32_data *data = dev->data;

	int ret;
	uint32_t freq;
	uint16_t psc;
	const struct device *clk;
	LL_TIM_InitTypeDef tim_init;
	LL_TIM_OC_InitTypeDef tim_ocinit;
	LL_TIM_BDTR_InitTypeDef brk_dt_init;

	if (!device_is_ready(config->currsmp)) {
		LOG_ERR("Current sampling device not ready");
		return -ENODEV;
	}

	/* configure pinmux */
	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		LOG_ERR("pinctrl setup failed (%d)", ret);
		return ret;
	}

	clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

	/* enable timer clock */
	ret = clock_control_on(clk, (clock_control_subsys_t *)&config->pclken);
	if (ret < 0) {
		LOG_ERR("Could not turn on timer clock (%d)", ret);
		return ret;
	}

	/* compute ARR */
	ret = stm32_tim_clk_get(&config->pclken, &freq);
	if (ret < 0) {
		return ret;
	}

	psc = 0U;
	do {
		data->period = __LL_TIM_CALC_ARR(
			freq, psc, CONFIG_SPINNER_SVPWM_STM32_PWM_FREQ * 2U);
		psc++;
	} while (data->period > UINT16_MAX);

	/* initialize timer
	 * NOTE: repetition counter set to 1, update will happen on underflow
	 */
	LL_TIM_StructInit(&tim_init);
	tim_init.CounterMode = LL_TIM_COUNTERMODE_CENTER_UP;
	tim_init.Autoreload = data->period;
	tim_init.RepetitionCounter = 1U;
	if (LL_TIM_Init(config->timer, &tim_init) != SUCCESS) {
		LOG_ERR("Could not initialize timer");
		return -EIO;
	}

	/* initialize OC for a, b, c channels */
	LL_TIM_OC_StructInit(&tim_ocinit);
	tim_ocinit.OCMode = LL_TIM_OCMODE_PWM1;
	tim_ocinit.CompareValue = data->period / 2U;

	if (LL_TIM_OC_Init(config->timer, LL_TIM_CHANNEL_CH1, &tim_ocinit) !=
	    SUCCESS) {
		LOG_ERR("Could not initialize timer OC for channel 1");
		return -EIO;
	}

	if (LL_TIM_OC_Init(config->timer, LL_TIM_CHANNEL_CH2, &tim_ocinit) !=
	    SUCCESS) {
		LOG_ERR("Could not initialize timer OC for channel 2");
		return -EIO;
	}

	if (LL_TIM_OC_Init(config->timer, LL_TIM_CHANNEL_CH3, &tim_ocinit) !=
	    SUCCESS) {
		LOG_ERR("Could not initialize timer OC for channel 3");
		return -EIO;
	}

	/* initialize OC for ADC trigger channel */
	tim_ocinit.OCMode = LL_TIM_OCMODE_PWM2;
	tim_ocinit.CompareValue = data->period - 1U;
	if (LL_TIM_OC_Init(config->timer, LL_TIM_CHANNEL_CH4, &tim_ocinit) !=
	    SUCCESS) {
		LOG_ERR("Could not initialize timer OC for channel 4");
		return -EIO;
	}

	LL_TIM_SetTriggerOutput(config->timer, LL_TIM_TRGO_OC4REF);

	/* enable pre-load on all OC channels */
	LL_TIM_OC_EnablePreload(config->timer, LL_TIM_CHANNEL_CH1);
	LL_TIM_OC_EnablePreload(config->timer, LL_TIM_CHANNEL_CH2);
	LL_TIM_OC_EnablePreload(config->timer, LL_TIM_CHANNEL_CH3);
	LL_TIM_OC_EnablePreload(config->timer, LL_TIM_CHANNEL_CH4);

	/* configure ADC sampling point (middle of the period) */
	LL_TIM_OC_SetCompareCH4(config->timer, data->period - 1U);

	/* setup break and dead-time if available */
	LL_TIM_BDTR_StructInit(&brk_dt_init);
	brk_dt_init.OSSRState = LL_TIM_OSSR_ENABLE;
	brk_dt_init.OSSIState = LL_TIM_OSSI_ENABLE;
	brk_dt_init.LockLevel = LL_TIM_LOCKLEVEL_1;
	/* TODO: add support for dead-time */
	brk_dt_init.DeadTime = 0U;
	brk_dt_init.BreakState = LL_TIM_BREAK_ENABLE;
	brk_dt_init.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
	brk_dt_init.Break2State = LL_TIM_BREAK2_ENABLE;
	if (LL_TIM_BDTR_Init(config->timer, &brk_dt_init) != SUCCESS) {
		LOG_ERR("Could not initialize timer break");
		return -EIO;
	}

	/* initialize enable GPIOs */
	for (size_t i = 0U; i < config->enable_len; i++) {
		const struct gpio_dt_spec *enable_gpio = &config->enable[i];

		if (!device_is_ready(enable_gpio->port)) {
			LOG_ERR("Enable GPIO not ready");
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(enable_gpio, GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			LOG_ERR("Could not configure enable GPIO");
			return ret;
		}
	}

	return 0;
}

PINCTRL_DT_INST_DEFINE(0);

#define ENABLE_GPIOS_ELEM(idx, _)                                              \
	GPIO_DT_SPEC_INST_GET_BY_IDX(0, enable_gpios, idx)

static const struct gpio_dt_spec enable_pins[] = {COND_CODE_1(
	DT_INST_NODE_HAS_PROP(0, enable_gpios),
	(LISTIFY(DT_INST_PROP_LEN(0, enable_gpios), ENABLE_GPIOS_ELEM, (,))),
	())};

static const struct svpwm_stm32_config svpwm_stm32_config = {
	.timer = (TIM_TypeDef *)DT_REG_ADDR(DT_INST_PARENT(0)),
	.pclken = {.bus = DT_CLOCKS_CELL(DT_INST_PARENT(0), bus),
		   .enr = DT_CLOCKS_CELL(DT_INST_PARENT(0), bits)},
	.enable_comp_outputs = DT_INST_PROP_OR(0, enable_comp_outputs, false),
	.t_dead = DT_INST_PROP_OR(0, t_dead_ns, 0),
	.t_rise = DT_INST_PROP_OR(0, t_rise_ns, 0),
	.currsmp = DEVICE_DT_GET(DT_INST_PHANDLE(0, currsmp)),
	.enable = enable_pins,
	.enable_len = ARRAY_SIZE(enable_pins),
	.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(0),
};

static struct svpwm_stm32_data svpwm_stm32_data;

DEVICE_DT_INST_DEFINE(0, &svpwm_stm32_init, NULL, &svpwm_stm32_data,
		      &svpwm_stm32_config, POST_KERNEL,
		      CONFIG_SPINNER_SVPWM_INIT_PRIORITY,
		      &svpwm_stm32_driver_api);
