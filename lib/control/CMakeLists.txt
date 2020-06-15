# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SPINNER_CLOOP)
  zephyr_library()
  zephyr_library_sources(cloop.c)
  zephyr_library_sources_ifdef(CONFIG_SPINNER_CLOOP_SHELL cloop_shell.c)
endif()

