/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>

#include <arm_math.h>

#include <spinner/drivers/currsmp.h>
#include <spinner/drivers/feedback.h>
#include <spinner/drivers/svpwm.h>

struct cloop {
	const struct device *currsmp;
	const struct device *feedback;
	const struct device *svpwm;
	arm_pid_instance_f32 pid_i_q;
	arm_pid_instance_f32 pid_i_d;
	float i_q_ref;
	float i_d_ref;
};

static struct cloop cloop;

/**
 * @brief Current regulation callback.
 *
 * This function is called after current sampling is completed.
 *
 * @warning It is called from the highest priority IRQ.
 */
static void regulate(void *ctx)
{
	struct currsmp_curr curr;
	float eangle, sin_eangle, cos_eangle;
	float i_alpha, i_beta;
	float i_q, i_d;
	float v_q, v_d;
	float v_alpha, v_beta;

	ARG_UNUSED(ctx);

	currsmp_get_currents(cloop.currsmp, &curr);
	eangle = feedback_get_eangle(cloop.feedback);
	arm_sin_cos_f32(eangle, &sin_eangle, &cos_eangle);

	/* i_a, i_b -> i_alpha, i_beta */
	arm_clarke_f32(curr.i_a, curr.i_b, &i_alpha, &i_beta);
	/* i_alpha, i_beta -> i_q, i_d */
	arm_park_f32(i_alpha, i_beta, &i_d, &i_q, sin_eangle, cos_eangle);

	/* PI (i_q, i_d -> v_q, v_d) */
	v_q = arm_pid_f32(&cloop.pid_i_q, cloop.i_q_ref - i_q);
	v_d = arm_pid_f32(&cloop.pid_i_d, cloop.i_d_ref - i_d);

	/* v_q, v_d -> v_alpha, v_beta */
	arm_inv_park_f32(v_d, v_q, &v_alpha, &v_beta, sin_eangle, cos_eangle);
	svpwm_set_phase_voltages(cloop.svpwm, v_alpha, v_beta);
}

static int cloop_init(void)
{
	cloop.currsmp = DEVICE_DT_GET(DT_NODELABEL(currsmp));
	cloop.svpwm = DEVICE_DT_GET(DT_NODELABEL(svpwm));
	cloop.feedback = DEVICE_DT_GET(DT_NODELABEL(feedback));

	cloop.i_q_ref = 0.0f;
	cloop.i_d_ref = 0.0f;

	cloop.pid_i_q.Kp = CONFIG_SPINNER_CLOOP_T_KP / 1000.0f;
	cloop.pid_i_q.Ki = CONFIG_SPINNER_CLOOP_T_KI / 1000.0f;
	cloop.pid_i_q.Kd = 0.0f;
	arm_pid_init_f32(&cloop.pid_i_q, 1);

	cloop.pid_i_d.Kp = CONFIG_SPINNER_CLOOP_F_KP / 1000.0f;
	cloop.pid_i_d.Ki = CONFIG_SPINNER_CLOOP_F_KI / 1000.0f;
	cloop.pid_i_d.Kd = 0.0f;
	arm_pid_init_f32(&cloop.pid_i_d, 1);

	currsmp_configure(cloop.currsmp, regulate, NULL);

	return 0;
}

SYS_INIT(cloop_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

/*******************************************************************************
 * Public
 ******************************************************************************/

void cloop_start(void)
{
	arm_pid_reset_f32(&cloop.pid_i_q);
	arm_pid_reset_f32(&cloop.pid_i_d);

	currsmp_start(cloop.currsmp);
	svpwm_start(cloop.svpwm);
}

void cloop_stop(void)
{
	svpwm_stop(cloop.svpwm);
	currsmp_stop(cloop.currsmp);
}

void cloop_set_ref(float i_d, float i_q)
{
	currsmp_pause(cloop.currsmp);
	cloop.i_d_ref = i_d;
	cloop.i_q_ref = i_q;
	currsmp_resume(cloop.currsmp);
}
