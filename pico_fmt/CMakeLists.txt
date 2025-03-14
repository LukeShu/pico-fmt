# Copyright (c) 2020  Raspberry Pi (Trading) Ltd.
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) 2025  Luke T. Shumaker
# SPDX-License-Identifier: BSD-3-Clause

if (NOT TARGET pico_fmt)
    include("../build-aux/pico_sdk_functions.cmake")
    include("../build-aux/apply_matrix.cmake")

    pico_add_library(pico_fmt)

    target_include_directories(pico_fmt_headers SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

    target_sources(pico_fmt INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/printf.c
            ${CMAKE_CURRENT_LIST_DIR}/convenience.c
    )
    target_link_libraries(pico_fmt INTERFACE pico_fmt_headers)

    if (PICO_SDK_TESTS_ENABLED)
        set(cfg_matrix
            # Toggle all the bools.
            "PICO_PRINTF_SUPPORT_FLOAT;[0;1]"
            "PICO_PRINTF_SUPPORT_EXPONENTIAL;[0;1]"
            "PICO_PRINTF_SUPPORT_LONG_LONG;[0;1]"
            "PICO_PRINTF_SUPPORT_PTRDIFF_T;[0;1]"

            # TODO: Spin the gauges.
            #"PICO_PRINTF_NTOA_BUFFER_SIZE;[0;32;128]"
            #"PICO_PRINTF_FTOA_BUFFER_SIZE;[0;32;128]"
            #"PICO_PRINTF_DEFAULT_FLOAT_PRECISION;[1;6;16]"
            #"PICO_PRINTF_MAX_FLOAT;[1;1e9]"
        )
        function(pico_fmt_add_test n defs)
            add_executable("test_suite_${n}" test/test_suite.c)
            target_link_libraries("test_suite_${n}" pico_fmt)
            target_compile_definitions("test_suite_${n}" PUBLIC "${defs}")
            add_test(
                NAME    "pico_fmt/test_suite_${n}"
                COMMAND valgrind --error-exitcode=2 "./test_suite_${n}"
            )
        endfunction()
        apply_matrix(pico_fmt_add_test "${cfg_matrix}")
    endif()
endif()
