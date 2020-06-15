#-------------------------------------------------------------------------------
# Spinner
#
# Copyright (c) 2021 Teslabs Engineering S.L.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(spinner LANGUAGES C VERSION 1.0.0)

#-------------------------------------------------------------------------------
# Options

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#-------------------------------------------------------------------------------
# Application

target_sources(app PRIVATE main.c)
