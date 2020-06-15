/**
 * @file
 *
 * Current loop API.
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_LIB_CONTROL_CLOOP_H_
#define _SPINNER_LIB_CONTROL_CLOOP_H_

/**
 * @defgroup spinner_lib_control Control APIs
 * @{
 * @}
 */

/**
 * @defgroup spinner_lib_control_cloop Current Loop API
 * @ingroup spinner_lib_control
 * @{
 */

/**
 * @brief Start current loop.
 */
void cloop_start(void);

/**
 * @brief Stop current loop.
 */
void cloop_stop(void);

/**
 * @brief Set current loop working point.
 *
 * @param[in] i_d i_d current value.
 * @param[in] i_q i_q current value.
 */
void cloop_set_ref(float i_d, float i_q);

/** @} */

#endif /* _SPINNER_LIB_CONTROL_CLOOP_H_ */
