/**
 * @file
 *
 * Current Sampling API.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_DRIVERS_CURRSMP_H_
#define _SPINNER_DRIVERS_CURRSMP_H_

#include <zephyr/types.h>
#include <device.h>

/**
 * @defgroup spinner_drivers Driver APIs
 * @{
 * @}
 */

/**
 * @defgroup spinner_drivers_currsmp Current Sampling API
 * @ingroup spinner_drivers
 * @{
 */

/** @brief Current sampling regulation callback. */
typedef void (*currsmp_regulation_cb_t)(void *ctx);

/** @brief Current sampling currents. */
struct currsmp_curr {
	/** Phase a current. */
	float i_a;
	/** Phase b current. */
	float i_b;
	/** Phase c current. */
	float i_c;
};

/** @cond INTERNAL_HIDDEN */

struct currsmp_driver_api {
	void (*configure)(const struct device *dev,
			  currsmp_regulation_cb_t regulation_cb, void *ctx);
	void (*get_currents)(const struct device *dev,
			     struct currsmp_curr *curr);
	void (*set_sector)(const struct device *dev, uint8_t sector);
	uint32_t (*get_smp_time)(const struct device *dev);
	void (*start)(const struct device *dev);
	void (*stop)(const struct device *dev);
	void (*pause)(const struct device *dev);
	void (*resume)(const struct device *dev);
};

/** @endcond */

/**
 * @brief Configure current sampling device.
 *
 * @note This function needs to be called before calling currsmp_start().
 *
 * @param[in] dev Current sampling device.
 * @param[in] regulation_cb Callback called on each regulation cycle.
 * @param[in] ctx Callback context.
 */
static inline void currsmp_configure(const struct device *dev,
				     currsmp_regulation_cb_t regulation_cb,
				     void *ctx)
{
	const struct currsmp_driver_api *api = dev->api;

	api->configure(dev, regulation_cb, ctx);
}

/**
 * @brief Get phase currents.
 *
 * @param[in] dev Current sampling device.
 * @param[out] curr Pointer where phase currents will be stored.
 */
static inline void currsmp_get_currents(const struct device *dev,
					struct currsmp_curr *curr)
{
	const struct currsmp_driver_api *api = dev->api;

	api->get_currents(dev, curr);
}

/**
 * @brief Set SV-PWM sector.
 *
 * @param[in] dev Current sampling device.
 * @param[in] sector SV-PWM sector.
 */
static inline void currsmp_set_sector(const struct device *dev, uint8_t sector)
{
	const struct currsmp_driver_api *api = dev->api;

	api->set_sector(dev, sector);
}

/**
 * @brief Obtain currents sampling time in nanoseconds.
 *
 * @param[in] dev Current sampling device.
 *
 * @return Sampling time in nanoseconds (zero indicates error).
 */
static inline uint32_t currsmp_get_smp_time(const struct device *dev)
{
	const struct currsmp_driver_api *api = dev->api;

	return api->get_smp_time(dev);
}

/**
 * @brief Start sampling currents.
 *
 * @param[in] dev Current sampling device.
 *
 * @see currsmp_stop()
 */
static inline void currsmp_start(const struct device *dev)
{
	const struct currsmp_driver_api *api = dev->api;

	api->start(dev);
}

/**
 * @brief Stop sampling currents.
 *
 * @param[in] dev Current sampling device.
 */
static inline void currsmp_stop(const struct device *dev)
{
	const struct currsmp_driver_api *api = dev->api;

	api->stop(dev);
}

/**
 * @brief Pause current sampling.
 *
 * @note This function can be used to prevent current sampling
 * to call the regulation callback, thus allowing to adjust
 * shared context.
 *
 * @param dev Current sampling device.
 *
 * @see currsmp_resume()
 */
static inline void currsmp_pause(const struct device *dev)
{
	const struct currsmp_driver_api *api = dev->api;

	api->pause(dev);
}

/**
 * @brief Resume current sampling.
 *
 * @param dev Current sampling device.
 */
static inline void currsmp_resume(const struct device *dev)
{
	const struct currsmp_driver_api *api = dev->api;

	api->resume(dev);
}

/** @} */

#endif /* _SPINNER_DRIVERS_CURRSMP_H_ */
