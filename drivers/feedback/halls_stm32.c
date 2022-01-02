/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_halls

#include <drivers/clock_control/stm32_clock_control.h>
#include <drivers/gpio.h>
#include <drivers/pinctrl.h>
#include <soc.h>

#include <stm32_ll_tim.h>

#include <spinner/drivers/feedback.h>
#include <spinner/utils/stm32_tim.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(halls_stm32, CONFIG_SPINNER_FEEDBACK_LOG_LEVEL);

/*******************************************************************************
 * Private
 ******************************************************************************/

struct halls_stm32_config {
	TIM_TypeDef *timer;
	struct stm32_pclken pclken;
	struct gpio_dt_spec h1;
	struct gpio_dt_spec h2;
	struct gpio_dt_spec h3;
	uint32_t irq;
	uint32_t phase_shift;
	const struct pinctrl_dev_config *pcfg;
};

struct halls_stm32_data {
	uint16_t eangle;
	uint8_t last_state;
	uint32_t tfreq;
	int32_t raw_speed;
};

static uint8_t halls_stm32_get_state(const struct device *dev)
{
	const struct halls_stm32_config *config = dev->config;

	return (uint8_t)gpio_pin_get_raw(config->h3.port, config->h3.pin)
		       << 2U |
	       (uint8_t)gpio_pin_get_raw(config->h2.port, config->h2.pin)
		       << 1U |
	       (uint8_t)gpio_pin_get_raw(config->h1.port, config->h1.pin);
}

ISR_DIRECT_DECLARE(timer_irq)
{
	const struct device *dev = DEVICE_DT_INST_GET(0);
	const struct halls_stm32_config *config = dev->config;
	struct halls_stm32_data *data = dev->data;

	uint8_t curr_state;
	uint16_t eangle = 0U;
	int8_t direction = 1;

	if (LL_TIM_IsActiveFlag_CC1(config->timer) == 0U) {
		return 0;
	}

	LL_TIM_ClearFlag_CC1(config->timer);

	curr_state = halls_stm32_get_state(dev);

	switch (curr_state) {
	case 5U:
		if (data->last_state == 4U) {
			eangle = 0;
		} else if (data->last_state == 1U) {
			eangle = 60;
			direction = -1;
		}

		break;
	case 1U:
		if (data->last_state == 5U) {
			eangle = 60;
		} else if (data->last_state == 3U) {
			eangle = 120;
			direction = -1;
		}
		break;
	case 3U:
		if (data->last_state == 1U) {
			eangle = 120;
		} else if (data->last_state == 2U) {
			eangle = 180;
			direction = -1;
		}
		break;
	case 2U:
		if (data->last_state == 3U) {
			eangle = 180;
		} else if (data->last_state == 6U) {
			eangle = 240;
			direction = -1;
		}
		break;
	case 6U:
		if (data->last_state == 2U) {
			eangle = 240;
		} else if (data->last_state == 4U) {
			eangle = 300;
			direction = -1;
		}
		break;
	case 4U:
		if (data->last_state == 6U) {
			eangle = 300;
		} else if (data->last_state == 5U) {
			eangle = 0;
			direction = -1;
		}
		break;
	default:
		__ASSERT(NULL, "Unexpected halls state: %d", curr_state);
		return 0;
	}

	eangle += config->phase_shift;

	data->eangle = eangle;
	data->last_state = curr_state;
	data->raw_speed =
		direction * (int32_t)LL_TIM_IC_GetCaptureCH1(config->timer);

	return 0;
}

/*******************************************************************************
 * API
 ******************************************************************************/

static float halls_stm32_get_eangle(const struct device *dev)
{
	struct halls_stm32_data *data = dev->data;

	return (float)data->eangle;
}

static float halls_stm32_get_speed(const struct device *dev)
{
	struct halls_stm32_data *data = dev->data;

	return (float)(data->tfreq / data->raw_speed / 6UL);
}

static const struct feedback_driver_api halls_stm32_driver_api = {
	.get_eangle = halls_stm32_get_eangle,
	.get_speed = halls_stm32_get_speed};

/*******************************************************************************
 * Init
 ******************************************************************************/

static int halls_stm32_init(const struct device *dev)
{
	const struct halls_stm32_config *config = dev->config;
	struct halls_stm32_data *data = dev->data;

	int ret;
	const struct device *clk;
	LL_TIM_InitTypeDef init;
	LL_TIM_ENCODER_InitTypeDef enc_init;
	uint8_t curr_state;

	/* configure pinmux */
	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		LOG_ERR("pinctrl setup failed (%d)", ret);
		return ret;
	}

	/* enable timer clock */
	clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

	ret = clock_control_on(clk, (clock_control_subsys_t *)&config->pclken);
	if (ret < 0) {
		LOG_ERR("Could not turn on timer clock (%d)", ret);
		return ret;
	}

	/* initialize timer */
	LL_TIM_StructInit(&init);
	if (LL_TIM_Init(config->timer, &init) != SUCCESS) {
		LOG_ERR("Could not initialize timer");
		return -EIO;
	}

	/* configure encoder (halls) mode */
	LL_TIM_SetClockSource(config->timer, LL_TIM_CLOCKSOURCE_INTERNAL);
	LL_TIM_IC_EnableXORCombination(config->timer);
	LL_TIM_SetTriggerInput(config->timer, LL_TIM_TS_TI1F_ED);

	LL_TIM_ENCODER_StructInit(&enc_init);
	enc_init.IC1ActiveInput = LL_TIM_ACTIVEINPUT_TRC;
	if (LL_TIM_ENCODER_Init(config->timer, &enc_init) != SUCCESS) {
		LOG_ERR("Could not initialize encoder mode");
		return -EIO;
	}

	/* configure CC unit and timer update source */
	LL_TIM_SetUpdateSource(config->timer, LL_TIM_UPDATESOURCE_COUNTER);
	LL_TIM_CC_EnableChannel(config->timer, LL_TIM_CHANNEL_CH1);
	LL_TIM_EnableIT_CC1(config->timer);

	/* store timer frequency (used for speed calculations) */
	ret = stm32_tim_clk_get(&config->pclken, &data->tfreq);
	if (ret < 0) {
		return ret;
	}

	/* check H1/H2/H3 GPIO readiness */
	if (!device_is_ready(config->h1.port) ||
	    !device_is_ready(config->h2.port) ||
	    !device_is_ready(config->h3.port)) {
		LOG_ERR("H1/H2/H3 GPIO device/s not ready");
		return -ENODEV;
	}

	/* initialize electrical angle */
	curr_state = halls_stm32_get_state(dev);
	switch (curr_state) {
	case 5U:
		data->eangle = 0U;
		break;
	case 1U:
		data->eangle = 60U;
		break;
	case 3U:
		data->eangle = 120U;
		break;
	case 2U:
		data->eangle = 180U;
		break;
	case 6U:
		data->eangle = 240U;
		break;
	case 4U:
		data->eangle = 300U;
		break;
	default:
		break;
	}

	data->eangle += config->phase_shift;
	data->last_state = curr_state;

	/* connect and enable timer IRQ */
	IRQ_DIRECT_CONNECT(DT_IRQ_BY_NAME(DT_INST_PARENT(0), global, irq),
			   DT_IRQ_BY_NAME(DT_INST_PARENT(0), global, priority),
			   timer_irq, 0);
	irq_enable(config->irq);

	return 0;
}

PINCTRL_DT_INST_DEFINE(0);

static const struct halls_stm32_config halls_stm32_config = {
	.timer = (TIM_TypeDef *)DT_REG_ADDR(DT_INST_PARENT(0)),
	.pclken = {.bus = DT_CLOCKS_CELL(DT_INST_PARENT(0), bus),
		   .enr = DT_CLOCKS_CELL(DT_INST_PARENT(0), bits)},
	.h1 = GPIO_DT_SPEC_INST_GET(0, h1_gpios),
	.h2 = GPIO_DT_SPEC_INST_GET(0, h2_gpios),
	.h3 = GPIO_DT_SPEC_INST_GET(0, h3_gpios),
	.irq = DT_IRQ_BY_NAME(DT_INST_PARENT(0), global, irq),
	.phase_shift = DT_INST_PROP(0, phase_shift),
	.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(0),
};

static struct halls_stm32_data halls_stm32_data;

DEVICE_DT_INST_DEFINE(0, &halls_stm32_init, NULL, &halls_stm32_data,
		      &halls_stm32_config, POST_KERNEL,
		      CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		      &halls_stm32_driver_api);
