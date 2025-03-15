// Copyright (c) 2014-2019  Marco Paland (info@paland.com)
// SPDX-License-Identifier: MIT
//
// Copyright (C) 2025  Luke T. Shumaker <lukeshu@lukeshu.com>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _PICO_FMT_INSTALL_H
#define _PICO_FMT_INSTALL_H

#include <stdarg.h>

#include "pico/fmt_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

// The interface your specifier must implement /////////////////////////////////

#define FMT_FLAG_ZEROPAD   (1U << 0U) // '0'
#define FMT_FLAG_LEFT      (1U << 1U) // '-'
#define FMT_FLAG_PLUS      (1U << 2U) // '+'
#define FMT_FLAG_SPACE     (1U << 3U) // ' '
#define FMT_FLAG_HASH      (1U << 4U) // '#'
#define FMT_FLAG_PRECISION (1U << 5U) // state->precision is set

enum fmt_size {
    FMT_SIZE_CHAR,      // "hh"
    FMT_SIZE_SHORT,     // "h"
    FMT_SIZE_DEFAULT,   // ""
    FMT_SIZE_LONG,      // "l"
    FMT_SIZE_LONG_LONG, // "ll"
};

struct _fmt_ctx;

struct fmt_state {
    // %[flags][width][.precision][size]specifier
    unsigned char flags;
    unsigned int width;
    unsigned int precision;
    enum fmt_size size;
    char specifier;

    va_list *args;

    struct _fmt_ctx *ctx;
};

typedef void (*fmt_specifier_t)(struct fmt_state *);

void fmt_state_putchar(struct fmt_state *state, char character);

/**
 * \brief The function signature that your custom handler must implement.
 */
typedef void (*fmt_specifier_t)(struct fmt_state *);

// Utilities for implementing the specifier ////////////////////////////////////

void fmt_state_putchar(struct fmt_state *state, char character);
void fmt_state_puts(struct fmt_state *state, const char *str); // no trailing newline
void fmt_state_vprintf(struct fmt_state *state, const char *format, va_list va) [[gnu::format(printf, 2, 0)]];
void fmt_state_printf(struct fmt_state *state, const char *format, ...) [[gnu::format(printf, 2, 3)]];

/**
 * \brief How many characters have been fmt_state_putchar()ed so far.
 *
 * When nested with fmt_state_printf(), the length is counted from the
 * beginning of the outermost printf.
 */
size_t fmt_state_len(struct fmt_state *state);

// To install the specifier ////////////////////////////////////////////////////

/**
 * Register `fn` to be called to handle `%character`.
 *
 * The character must be a valid ASCII character that is printing
 * (excludes whitespace and control-codes) and non-numeric; otherwise
 * the call to fmt_install() is ignored.
 *
 * This may re-define existing specifier characters.  What happens if
 * the character clashes with an existing non-specifier character that
 * is used in parsing (flag, size, or numeric) is not well-defined.
 */
void fmt_install(char character, fmt_specifier_t fn);

#ifdef __cplusplus
}
#endif

#endif
