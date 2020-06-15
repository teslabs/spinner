# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_SPINNER_CURRSMP currsmp)
add_subdirectory_ifdef(CONFIG_SPINNER_FEEDBACK feedback)
add_subdirectory_ifdef(CONFIG_SPINNER_SVPWM svpwm)
