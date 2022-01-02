/**
 * @file
 *
 * SV-PWM API.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_DRIVERS_SVPWM_H_
#define _SPINNER_DRIVERS_SVPWM_H_

#include <device.h>
#include <zephyr/types.h>

/**
 * @defgroup spinner_drivers_svpwm SV-PWM API
 * @ingroup spinner_drivers
 * @{
 */

/** @cond INTERNAL_HIDDEN */

struct svpwm_driver_api {
	void (*start)(const struct device *dev);
	void (*stop)(const struct device *dev);
	void (*set_phase_voltages)(const struct device *dev, float v_alpha,
				   float v_beta);
};

/** @endcond */

/**
 * @brief Start the SV-PWM controller.
 *
 * @note Current sampling device must be started prior to SV-PWM, since SV-PWM
 * is the responsible to trigger current sampling measurements.
 *
 * @param[in] dev SV-PWM device.
 */
static inline void svpwm_start(const struct device *dev)
{
	const struct svpwm_driver_api *api = dev->api;

	api->start(dev);
}

/**
 * @brief Stop the SV-PWM controller.
 *
 * @param[in] dev SV-PWM device.
 */
static inline void svpwm_stop(const struct device *dev)
{
	const struct svpwm_driver_api *api = dev->api;

	api->stop(dev);
}

/**
 * @brief Set phase voltages.
 *
 * @param[in] dev SV-PWM device.
 * @param[in] v_alpha Alpha voltage.
 * @param[in] v_beta Beta voltage.
 */
static inline void svpwm_set_phase_voltages(const struct device *dev,
					    float v_alpha, float v_beta)
{
	const struct svpwm_driver_api *api = dev->api;

	api->set_phase_voltages(dev, v_alpha, v_beta);
}

/** @} */

#endif /* _SPINNER_DRIVERS_SVPWM_H_ */
