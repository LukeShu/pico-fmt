# Copyright (c) 2020  Raspberry Pi (Trading) Ltd.
# SPDX-License-Identifier: BSD-3-Clause

################################################################################
# From pico-sdk:src/CMakeLists.txt
################################################################################

# create an INTERFACE library named target, and define LIB_TARGET=1 (upper case) as a compile option
# and make it dependent on a pre-existing corresponding _headers library
# optional arg NOFLAG will skip the LIB_TARGET definition
function(pico_add_impl_library target)
    add_library(${target} INTERFACE)
    string(TOUPPER ${target} TARGET_UPPER)
    if (${ARGC} GREATER 1)
        if (NOT "${ARGV1}" STREQUAL "NOFLAG")
            message(FATAL_ERROR "Unknown parameter ${ARGV1}")
        endif()
    else()
        target_compile_definitions(${target} INTERFACE LIB_${TARGET_UPPER}=1)
    endif()
    target_link_libraries(${target} INTERFACE ${target}_headers)
endfunction()

# create an INTERFACE library named target along with associated header, and define LIB_TARGET=1 (upper case) as a compile option
# optional arg NOFLAG will skip the LIB_TARGET definition
function(pico_add_library target)
    add_library(${target}_headers INTERFACE)
    pico_add_impl_library(${target} ${ARGN})
endfunction()

# add a link option to wrap the given function name; i.e. -Wl:wrap=FUNCNAME for gcc
function(pico_wrap_function TARGET FUNCNAME)
    target_link_options(${TARGET} INTERFACE "LINKER:--wrap=${FUNCNAME}")
endfunction()
