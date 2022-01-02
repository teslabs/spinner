/*
 * Copyright (c) 2021 Teslabs Engineering S.L.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <shell/shell.h>

#include <spinner/control/cloop.h>

static int cmd_cloop_start(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(shell);
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	cloop_start();

	return 0;
}

static int cmd_cloop_stop(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(shell);
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	cloop_stop();

	return 0;
}

static int cmd_cloop_set(const struct shell *shell, size_t argc, char **argv)
{
	if (argc != 2) {
		shell_help(shell);
		return -EINVAL;
	}

	/* NOTE: i_d = 0, assuming PMSM */
	cloop_set_ref(0.0f, strtof(argv[1], NULL));

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cloop,
	SHELL_CMD(start, NULL, "Start current regulation loop",
		  cmd_cloop_start),
	SHELL_CMD(stop, NULL, "Stop current regulation loop", cmd_cloop_stop),
	SHELL_CMD(set, NULL, "Set current regulation loop target",
		  cmd_cloop_set),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(cloop, &sub_cloop, "Current Loop Control", NULL);
