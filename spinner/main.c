/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <spinner/control/cloop.h>

int main(void)
{
	cloop_start();
	cloop_set_ref(0.0f, 0.25f);

	return 0;
}
