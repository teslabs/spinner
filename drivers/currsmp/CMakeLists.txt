# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_SPINNER_CURRSMP_SHUNT_STM32 currsmp_shunt_stm32.c)

