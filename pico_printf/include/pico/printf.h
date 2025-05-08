/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2025  Luke T. Shumaker <lukeshu@lukeshu.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PICO_PRINTF_H
#define _PICO_PRINTF_H

#include "pico.h"
#include <stdio.h>
#include <stdarg.h>

/** \file printf.h
 * \defgroup pico_printf pico_printf
 *
 * \brief Wrapper for weak-inclusion of \ref pico_fmt
 */

#ifdef __cplusplus
extern "C" {
#endif

// PICO_CONFIG: PICO_PRINTF_ALWAYS_INCLUDED, Whether to always include printf code even if only called weakly (by panic), type=bool, default=1 in debug build 0 otherwise, group=pico_printf
#ifndef PICO_PRINTF_ALWAYS_INCLUDED
    #ifndef NDEBUG
        #define PICO_PRINTF_ALWAYS_INCLUDED 1
    #else
        #define PICO_PRINTF_ALWAYS_INCLUDED 0
    #endif
#endif

#if LIB_PICO_PRINTF_PICO
    // weak raw printf may be a puts if printf has not been called,
    // so that we can support gc of printf when it isn't called
    //
    // it is called raw to distinguish it from the regular printf which
    // is in stdio.c and does mutex protection
    #if !PICO_PRINTF_ALWAYS_INCLUDED
        bool __printflike(1, 0) weak_raw_printf(const char *fmt, ...);
        bool weak_raw_vprintf(const char *fmt, va_list args);
    #else
        #define weak_raw_printf(...) ({printf(__VA_ARGS__); true;})
        #define weak_raw_vprintf(fmt,va) ({vprintf(fmt,va); true;})
    #endif

    /**
     * \brief printf with output function
     * You may use this as dynamic alternative to printf() with its fixed _putchar() output
     * \param out An output function which takes one character and an argument pointer
     * \param arg An argument pointer for user data passed to output function
     * \param format A string that specifies the format of the output
     * \return The number of characters that are sent to the output function, not counting the terminating null character
     */
    int vfctprintf(void (*out)(char character, void *arg), void *arg, const char *format, va_list va);
#else

    #define weak_raw_printf(...) ({printf(__VA_ARGS__); true;})
    #define weak_raw_vprintf(fmt,va) ({vprintf(fmt,va); true;})

#endif

#ifdef __cplusplus
}
#endif

#endif
