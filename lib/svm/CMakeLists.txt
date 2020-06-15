# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_SPINNER_SVM)
  zephyr_library()
  zephyr_library_sources(svm.c)
endif()

