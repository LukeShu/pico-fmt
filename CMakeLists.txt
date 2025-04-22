# CMakeLists.txt - For working on pico-fmt itself; users should not use this file
#
# Copyright (c) 2025  Luke T. Shumaker
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13...3.27)
project(pico_fmt C)

set(PICO_SDK_TESTS_ENABLED 1)

include(CTest)
add_subdirectory("pico_fmt")
add_subdirectory("pico_printf")
