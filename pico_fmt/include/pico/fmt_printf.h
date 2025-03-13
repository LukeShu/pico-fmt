// Copyright (c) 2014-2019  Marco Paland (info@paland.com)
// SPDX-License-Identifier: MIT
//
// Copyright (c) 2020  Raspberry Pi (Trading) Ltd.
// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2025  Luke T. Shumaker <lukeshu@lukeshu.com>
// SPDX-License-Identifier: BSD-3-Clause
//
///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources.
//        Use this instead of bloated standard/newlib printf.
//        These routines are thread safe and reentrant.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _PICO_FMT_PRINTF_H
#define _PICO_FMT_PRINTF_H

#include <stdarg.h> /* for va_list */
#include <stddef.h> /* for size_t */

/** \file fmt_printf.h
 *  \defgroup pico_fmt_printf pico_fmt_printf
 *
 * \brief Compact replacement for printf by Marco Paland (info@paland.com)
 */

#ifdef __cplusplus
extern "C" {
#endif

// Core API ////////////////////////////////////////////////////////////////////

/**
 * \brief An output function
 */
typedef void (*fmt_fct_t)(char character, void *arg);

/**
 * \brief vprintf with output function
 *
 * \param out An output function which takes one character and an argument pointer
 * \param arg An argument pointer for user data passed to output function
 * \param format A string that specifies the format of the output
 * \return The number of characters that are sent to the output function, not counting the terminating null character
 */
int fmt_vfctprintf(fmt_fct_t out, void *arg, const char *format, va_list va);

// Convenience functions ///////////////////////////////////////////////////////

int fmt_fctprintf(fmt_fct_t out, void *arg, const char *format, ...);

int fmt_vsnprintf(char *buffer, size_t count, const char *format, va_list);
int fmt_snprintf(char *buffer, size_t count, const char *format, ...);
int fmt_vsprintf(char *buffer, const char *format, va_list);
int fmt_sprintf(char *buffer, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // _PICO_FMT_PRINTF_H_
