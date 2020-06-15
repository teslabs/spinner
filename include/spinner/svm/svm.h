/**
 * @file
 *
 * Space Vector Modulation (SVM).
 *
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SPINNER_LIB_SVM_SVM_H_
#define _SPINNER_LIB_SVM_SVM_H_

#include <stdint.h>

/**
 * @defgroup spinner_control_svm Space Vector Modulation (SVM) API
 * @{
 */

/** @brief SVM duty cycles. */
typedef struct {
	/** A channel duty cycle. */
	float a;
	/** B channel duty cycle. */
	float b;
	/** C channel duty cycle. */
	float c;
	/** Maximum duty cycle of a, b, c. */
	float max;
} svm_duties_t;

/** @brief SVM state. */
typedef struct svm {
	/** SVM sector. */
	uint8_t sector;
	/** Duty cycles. */
	svm_duties_t duties;
	/** Minimum allowed duty cycle. */
	float d_min;
	/** Maximum allowed duty cycle. */
	float d_max;
} svm_t;

/**
 * @brief Initialize SVM.
 *
 * @param[in] svm SVM instance.
 */
void svm_init(svm_t *svm);

/**
 * @brief Set v_alpha and v_beta.
 *
 * @param[in] svm SVM instance.
 * @param[in] va v_alpha value.
 * @param[in] vb v_beta value.
 */
void svm_set(svm_t *svm, float va, float vb);

/** @} */

#endif /* _SPINNER_LIB_SVM_SVM_H_ */
