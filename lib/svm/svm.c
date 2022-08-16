/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/util.h>

#include <arm_math.h>

#include <spinner/svm/svm.h>

/*******************************************************************************
 * Private
 ******************************************************************************/

/** Value sqrt(3). */
#define SQRT_3 1.7320508075688773f

/**
 * @brief Obtain sector based on a, b, c vector values.
 *
 * @param[in] a a component value.
 * @param[in] b b component value.
 * @param[in] c c component value.

 * @return Sector (1...6).
 */
static uint8_t get_sector(float a, float b, float c)
{
	uint8_t sector = 0U;

	if (c < 0.0f) {
		if (a < 0.0f) {
			sector = 2U;
		} else {
			if (b < 0.0f) {
				sector = 6U;
			} else {
				sector = 1U;
			}
		}
	} else {
		if (a < 0.0f) {
			if (b <= 0.0f) {
				sector = 4U;
			} else {
				sector = 3U;
			}
		} else {
			sector = 5U;
		}
	}

	return sector;
}

/*******************************************************************************
 * Public
 ******************************************************************************/

void svm_init(svm_t *svm)
{
	svm->sector = 0U;

	svm->duties.a = 0.0f;
	svm->duties.b = 0.0f;
	svm->duties.c = 0.0f;

	svm->d_min = 0.0f;
	svm->d_max = 1.0f;
}

void svm_set(svm_t *svm, float va, float vb)
{
	float a, b, c, mod;
	float x, y, z;

	/* limit maximum amplitude to avoid distortions */
	(void)arm_sqrt_f32(va * va + vb * vb, &mod);
	if (mod > SQRT_3 / 2.0f) {
		va = va / mod * (SQRT_3 / 2.0f);
		vb = vb / mod * (SQRT_3 / 2.0f);
	}

	a = va - 1.0f / SQRT_3 * vb;
	b = 2.0f / SQRT_3 * vb;
	c = -(a + b);

	svm->sector = get_sector(a, b, c);

	switch (svm->sector) {
	case 1U:
		x = a;
		y = b;
		z = 1.0f - (x + y);

		svm->duties.a = x + y + z * 0.5f;
		svm->duties.b = y + z * 0.5f;
		svm->duties.c = z * 0.5f;

		break;
	case 2U:
		x = -c;
		y = -a;
		z = 1.0f - (x + y);

		svm->duties.a = x + z * 0.5f;
		svm->duties.b = x + y + z * 0.5f;
		svm->duties.c = z * 0.5f;

		break;
	case 3U:
		x = b;
		y = c;
		z = 1.0f - (x + y);

		svm->duties.a = z * 0.5f;
		svm->duties.b = x + y + z * 0.5f;
		svm->duties.c = y + z * 0.5f;

		break;
	case 4U:
		x = -a;
		y = -b;
		z = 1.0f - (x + y);

		svm->duties.a = z * 0.5f;
		svm->duties.b = x + z * 0.5f;
		svm->duties.c = x + y + z * 0.5f;

		break;
	case 5U:
		x = c;
		y = a;
		z = 1.0f - (x + y);

		svm->duties.a = y + z * 0.5f;
		svm->duties.b = z * 0.5f;
		svm->duties.c = x + y + z * 0.5f;

		break;
	case 6U:
		x = -b;
		y = -c;
		z = 1.0f - (x + y);

		svm->duties.a = x + y + z * 0.5f;
		svm->duties.b = z * 0.5f;
		svm->duties.c = x + z * 0.5f;

		break;
	default:
		break;
	}

	svm->duties.a = CLAMP(svm->duties.a, svm->d_min, svm->d_max);
	svm->duties.b = CLAMP(svm->duties.b, svm->d_min, svm->d_max);
	svm->duties.c = CLAMP(svm->duties.c, svm->d_min, svm->d_max);
}
