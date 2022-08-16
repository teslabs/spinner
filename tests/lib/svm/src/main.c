/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>

#include <zephyr/ztest.h>

#include <spinner/svm/svm.h>

/** Value of sqrt(3). */
#define SQRT_3 1.7320508075688773f

/** @brief Test that two floats are almost equal (works for small numbers). */
#define ALMOST_EQUAL(a, b) (fabs((a) - (b)) < 1.0e-5)

/**
 * @brief Test that SV-PWM modulator works as expected.
 *
 * Space vector angular values from 0 to 360 degrees are tested in steps of 30
 * so that we can compare against exact results while testing all sectors. The
 * svm_set() input values are computed as follows:
 *
 * - A space vector angle is chosen [0..360), which determines the sector
 * - Space vector a, b, c components are computed
 * - a, b, c components are transformed to the alpha-beta space
 *
 * Note that in most cases a vector with a module 1 is used for simplicity
 * (svm_set() performs amplitude limitation, also tested). Angles that are at
 * the edge of two sectors (0, 60, 120, 180, 240, 300) are tested to be in
 * either of the adjacent sectors, since sector determination based on sign
 * threshold may carry floating point rounding errors.
 */
ZTEST(svm, test_api)
{
	svm_t svm;

	/* initial state */
	svm_init(&svm);
	zassert_equal(svm.duties.a, 0.0f, NULL);
	zassert_equal(svm.duties.b, 0.0f, NULL);
	zassert_equal(svm.duties.c, 0.0f, NULL);
	zassert_equal(svm.d_min, 0.0f, NULL);
	zassert_equal(svm.d_max, 1.0f, NULL);
	zassert_equal(svm.sector, 0U, NULL);

	/* 0 degrees (mod sqrt(3) / 2) */
	svm_set(&svm, SQRT_3 / 2.0f, 0.0f);
	zassert_true((svm.sector == 1U) || (svm.sector == 6U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f - SQRT_3 / 4.0f), NULL);

	/* 0 degrees (amplitude is limited here, as all others that follow) */
	svm_set(&svm, 1.0f, 0.0f);
	zassert_true((svm.sector == 1U) || (svm.sector == 6U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f - SQRT_3 / 4.0f), NULL);

	/* 30 degres */
	svm_set(&svm, SQRT_3 / 2.0f, 0.5f);
	zassert_equal(svm.sector, 1U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 1.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.0f), NULL);

	/* 60 degrees */
	svm_set(&svm, 0.5f, SQRT_3 / 2.0f);
	zassert_true((svm.sector == 1U) || (svm.sector == 2U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f - SQRT_3 / 4.0f), NULL);

	/* 90 degrees */
	svm_set(&svm, 0.0f, 1.0f);
	zassert_equal(svm.sector, 2U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 1.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.0f), NULL);

	/* 120 degrees */
	svm_set(&svm, -0.5f, 3.0f / (2.0f * SQRT_3));
	zassert_true((svm.sector == 2U) || (svm.sector == 3U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f - SQRT_3 / 4.0f), NULL);

	/* 150 degrees */
	svm_set(&svm, -SQRT_3 / 2.0f, 0.5f);
	zassert_equal(svm.sector, 3U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 1.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f), NULL);

	/* 180 degrees */
	svm_set(&svm, -1.0f, 0.0f);
	zassert_true((svm.sector == 3U) || (svm.sector == 4U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f + SQRT_3 / 4.0f), NULL);

	/* 210 degrees */
	svm_set(&svm, -SQRT_3 / 2.0f, -0.5f);
	zassert_equal(svm.sector, 4U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 1.0f), NULL);

	/* 240 degrees */
	svm_set(&svm, -0.5f, -SQRT_3 / 2.0f);
	zassert_true((svm.sector == 4U) || (svm.sector == 5U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f + SQRT_3 / 4.0f), NULL);

	/* 270 degrees */
	svm_set(&svm, 0.0f, -1.0f);
	zassert_equal(svm.sector, 5U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 1.0f), NULL);

	/* 300 degrees */
	svm_set(&svm, 0.5f, -SQRT_3 / 2.0f);
	zassert_true((svm.sector == 5U) || (svm.sector == 6U), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 0.5f + SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.5f - SQRT_3 / 4.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f + SQRT_3 / 4.0f), NULL);

	/* 330 degrees */
	svm_set(&svm, SQRT_3 / 2.0f, -0.5f);
	zassert_equal(svm.sector, 6U, NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.a, 1.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.b, 0.0f), NULL);
	zassert_true(ALMOST_EQUAL(svm.duties.c, 0.5f), NULL);
}

ZTEST_SUITE(svm, NULL, NULL, NULL, NULL, NULL);
