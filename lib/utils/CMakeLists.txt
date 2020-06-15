# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

if (CONFIG_SPINNER_UTILS_STM32)
  zephyr_library()
  zephyr_library_sources(stm32_tim.c stm32_adc.c)
endif()
