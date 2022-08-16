/**
 * @file
 *
 * Feedback API.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_DRIVERS_FEEDBACK_H_
#define _SPINNER_DRIVERS_FEEDBACK_H_

#include <zephyr/device.h>
#include <zephyr/types.h>

/**
 * @defgroup spinner_drivers_feedback Feedback API
 * @ingroup spinner_drivers
 * @{
 */

/** @cond INTERNAL_HIDDEN */

struct feedback_driver_api {
	float (*get_eangle)(const struct device *dev);
	float (*get_speed)(const struct device *dev);
};

/** @endcond */

/**
 * @brief Get electrical angle.
 *
 * @param dev Feedback instance.
 * @return Electrical angle.
 */
static inline float feedback_get_eangle(const struct device *dev)
{
	const struct feedback_driver_api *api = dev->api;

	return api->get_eangle(dev);
}

/**
 * @brief Get speed.
 *
 * @param dev Feedback instance.
 * @return Speed.
 */
static inline float feedback_get_speed(const struct device *dev)
{
	const struct feedback_driver_api *api = dev->api;

	return api->get_speed(dev);
}

/** @} */

#endif /* _SPINNER_DRIVERS_FEEDBACK_H_ */
