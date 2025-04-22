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
// \brief Tiny printf, sprintf and (v)snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources. These routines are thread
//        safe and reentrant!
//        Use this instead of the bloated standard/newlib printf cause these use
//        malloc for printf (and may not be thread safe).
//
///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>

#include "pico/fmt_install.h"
#include "pico/fmt_printf.h"

// PICO_CONFIG: PICO_PRINTF_FTOA_BUFFER_SIZE, Define printf ftoa buffer size, min=0, max=128, default=32, group=pico_printf
// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
#ifndef PICO_PRINTF_FTOA_BUFFER_SIZE
#define PICO_PRINTF_FTOA_BUFFER_SIZE 32U
#endif

// PICO_CONFIG: PICO_PRINTF_SUPPORT_FLOAT, Enable floating point printing, type=bool, default=1, group=pico_printf
// support for the floating point type (%f)
#ifndef PICO_PRINTF_SUPPORT_FLOAT
#define PICO_PRINTF_SUPPORT_FLOAT 1
#endif

// PICO_CONFIG: PICO_PRINTF_SUPPORT_EXPONENTIAL, Enable exponential floating point printing, type=bool, default=1, group=pico_printf
// support for exponential floating point notation (%e/%g)
#ifndef PICO_PRINTF_SUPPORT_EXPONENTIAL
#define PICO_PRINTF_SUPPORT_EXPONENTIAL 1
#endif

// PICO_CONFIG: PICO_PRINTF_DEFAULT_FLOAT_PRECISION, Define default floating point precision, min=1, max=16, default=6, group=pico_printf
#ifndef PICO_PRINTF_DEFAULT_FLOAT_PRECISION
#define PICO_PRINTF_DEFAULT_FLOAT_PRECISION 6U
#endif

// PICO_CONFIG: PICO_PRINTF_MAX_FLOAT, Define the largest float suitable to print with %f, min=1, max=1e9, default=1e9, group=pico_printf
#ifndef PICO_PRINTF_MAX_FLOAT
#define PICO_PRINTF_MAX_FLOAT 1e9
#endif

// PICO_CONFIG: PICO_PRINTF_SUPPORT_LONG_LONG, Enable support for long long types (%llu or %p), type=bool, default=1, group=pico_printf
#ifndef PICO_PRINTF_SUPPORT_LONG_LONG
#define PICO_PRINTF_SUPPORT_LONG_LONG 1
#endif

// PICO_CONFIG: PICO_PRINTF_SUPPORT_PTRDIFF_T, Enable support for the ptrdiff_t type (%t), type=bool, default=1, group=pico_printf
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
#ifndef PICO_PRINTF_SUPPORT_PTRDIFF_T
#define PICO_PRINTF_SUPPORT_PTRDIFF_T 1
#endif

// import float.h for DBL_MAX
#if PICO_PRINTF_SUPPORT_FLOAT
#include <float.h>
#endif

// utilities //////////////////////////////////////////////////////////////////

struct _fmt_ctx {
    fmt_fct_t fct;
    void *arg;
    size_t idx;
};

inline void fmt_state_putchar(struct fmt_state *state, char character) {
    if (state->ctx->fct) {
        state->ctx->fct(character, state->ctx->arg);
    }
    state->ctx->idx++;
}

inline void fmt_state_puts(struct fmt_state *state, const char *str) {
    while (*str)
        fmt_state_putchar(state, *(str++));
}

inline size_t fmt_state_len(struct fmt_state *state) {
    return state->ctx->idx;
}

#define array_len(ary) (sizeof(ary) / sizeof(ary[0]))
#define max(a, b)      ((a) > (b) ? (a) : (b))

// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by 'maxsize'
static inline unsigned int _strnlen_s(const char *str, size_t maxsize) {
    const char *s;
    for (s = str; *s && maxsize--; ++s)
        ;
    return (unsigned int) (s - str);
}

// The `~` operator, but without C promoting it to an int, which would
// then trip -Wconversion.
static inline fmt_flags flipflag(fmt_flags x) {
    return ~x;
}

// internal test if char is a digit (0-9)
// \return true if char is a digit
static inline bool _is_digit(char ch) {
    return (ch >= '0') && (ch <= '9');
}

static inline bool _is_upper(char ch) {
    return (ch >= 'A') && (ch <= 'Z');
}

// internal ASCII string to unsigned int conversion
static unsigned int _atoi(const char **str) {
    unsigned int i = 0U;
    while (_is_digit(**str)) {
        i = i * 10U + (unsigned int) (*((*str)++) - '0');
    }
    return i;
}

#if PICO_PRINTF_SUPPORT_FLOAT

// output the specified string in reverse, taking care of any zero-padding
static void _out_rev(struct fmt_state *state, const char *buf, size_t len) {
    const size_t start_idx = fmt_state_len(state);

    // pad spaces up to given width
    if (!(state->flags & FMT_FLAG_LEFT) && !(state->flags & FMT_FLAG_ZEROPAD)) {
        for (size_t i = len; i < state->width; i++) {
            fmt_state_putchar(state, ' ');
        }
    }

    // reverse string
    while (len) {
        fmt_state_putchar(state, buf[--len]);
    }

    // append pad spaces up to given width
    if (state->flags & FMT_FLAG_LEFT) {
        while (fmt_state_len(state) - start_idx < state->width) {
            fmt_state_putchar(state, ' ');
        }
    }
}
#endif

// int ////////////////////////////////////////////////////////////////////////

static void _ntoa_intro(struct fmt_state *state, unsigned base, unsigned ndigits, int sign) {
    unsigned nextra = 0;
    switch (base) {
        case 2:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0)
                nextra = 2; // "0b"
            break;
        case 8:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0)
                nextra = 1; // "0"
            break;
        case 10:
            if (state->flags & (FMT_FLAG_PLUS | FMT_FLAG_SPACE))
                nextra = 1; // "+" or "-" or " "
            else if (sign < 0)
                nextra = 1; // "-"
            break;
        case 16:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0)
                nextra = 2; // "0x"
            break;
    }

    if (state->flags & FMT_FLAG_PRECISION)
        // ignore '0' flag when precision is given
        state->flags &= flipflag(FMT_FLAG_ZEROPAD);

    // emit leading spaces
    if (state->width &&
        !(state->flags & FMT_FLAG_LEFT) &&
        !(state->flags & FMT_FLAG_ZEROPAD))
        for (unsigned i = max(state->precision, ndigits) + nextra; i < state->width; i++)
            fmt_state_putchar(state, ' ');

    // emit base or sign
    switch (base) {
        case 2:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0) {
                fmt_state_putchar(state, '0');
                fmt_state_putchar(state, 'b');
            }
            break;
        case 8:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0)
                fmt_state_putchar(state, '0');
            break;
        case 10:
            if (sign < 0)
                fmt_state_putchar(state, '-');
            else if (state->flags & FMT_FLAG_PLUS)
                fmt_state_putchar(state, '+');
            else if (state->flags & FMT_FLAG_SPACE)
                fmt_state_putchar(state, ' ');
            break;
        case 16:
            if ((state->flags & FMT_FLAG_HASH) && sign != 0) {
                fmt_state_putchar(state, '0');
                fmt_state_putchar(state, state->specifier);
            }
            break;
    }

    // emit leading zeroes
    if (state->flags & FMT_FLAG_PRECISION) {
        for (unsigned i = ndigits; i < state->precision; i++)
            fmt_state_putchar(state, '0');
    } else if (state->width &&
               !(state->flags & FMT_FLAG_LEFT) &&
               (state->flags & FMT_FLAG_ZEROPAD)) {
        for (unsigned i = ndigits + nextra; i < state->width; i++)
            fmt_state_putchar(state, '0');
    } else if (sign == 0) {
        // always have at least one '0' digit, unless precision told us otherwise
        fmt_state_putchar(state, '0');
    }
}

static void _ntoa_outro(struct fmt_state *state, size_t start_idx) {
    // emit trailing spaces
    for (size_t len = fmt_state_len(state) - start_idx; len < state->width; len++)
        fmt_state_putchar(state, ' ');
}

#define _define_ntoa(TYP, SUF)                                                       \
    static void _ntoa##SUF(struct fmt_state *state, unsigned TYP absval,             \
                           bool negative, unsigned base) {                           \
        const size_t start_idx = fmt_state_len(state);                               \
        unsigned ndigits = 0;                                                        \
        unsigned TYP div;                                                            \
        if (absval) {                                                                \
            /* This is O(log(absval)); there are `__builtin_clz`-based O(1)          \
             * ways to do this, but when I tried them they bloated the               \
             * code-size too much.  And this function as a whole is already          \
             * O(log(absval)) anyway because of actually printing the digits.  */    \
            ndigits = 1;                                                             \
            div = 1;                                                                 \
            while (absval / div >= base) {                                           \
                div *= base;                                                         \
                ndigits++;                                                           \
            }                                                                        \
        }                                                                            \
                                                                                     \
        /* emit leading whitespace, base/sign, and leading zeros */                  \
        _ntoa_intro(state, base, ndigits, absval ? (negative ? -1 : 1) : 0);         \
                                                                                     \
        /* emit the main number */                                                   \
        for (unsigned i = 0; i < ndigits; i++) {                                     \
            char digit = (char) (absval / div);                                      \
            absval %= div;                                                           \
            div /= base;                                                             \
            char c;                                                                  \
            if (digit < 10)                                                          \
                c = (char) '0' + digit;                                              \
            else                                                                     \
                c = (char) ((_is_upper(state->specifier) ? 'A' : 'a') + digit - 10); \
            fmt_state_putchar(state, c);                                             \
        }                                                                            \
                                                                                     \
        /* emit trailing spaces */                                                   \
        _ntoa_outro(state, start_idx);                                               \
    }

_define_ntoa(int, );

#if __SIZEOF_LONG__ == __SIZEOF_INT__
#define _ntoal _ntoa
#else
_define_ntoa(long, l);
#endif

#if PICO_PRINTF_SUPPORT_LONG_LONG
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
#define _ntoall _ntoal
#else
_define_ntoa(long long, ll);
#endif
#endif

// float //////////////////////////////////////////////////////////////////////

#if PICO_PRINTF_SUPPORT_FLOAT

#define is_nan __builtin_isnan

static bool _float_special(struct fmt_state *state, double value) {
    // test for special values
    if (is_nan(value)) {
        _out_rev(state, "nan", 3);
        return true;
    }
    if (value < -DBL_MAX) {
        _out_rev(state, "fni-", 4);
        return true;
    }
    if (value > DBL_MAX) {
        _out_rev(state, (state->flags & FMT_FLAG_PLUS) ? "fni+" : "fni", (state->flags & FMT_FLAG_PLUS) ? 4U : 3U);
        return true;
    }
    return false;
}

// internal ftoa for fixed decimal floating point
static void _ftoa(struct fmt_state *state, double value) {
    char buf[PICO_PRINTF_FTOA_BUFFER_SIZE];
    size_t len = 0U;
    double diff = 0.0;

    // powers of 10
    static const double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

    // check for NaN and special values
    if (_float_special(state, value))
        return;

    // test for negative
    bool negative = false;
    if (value < 0) {
        negative = true;
        value = 0 - value;
    }

    // set default precision, if not set explicitly
    if (!(state->flags & FMT_FLAG_PRECISION)) {
        state->precision = PICO_PRINTF_DEFAULT_FLOAT_PRECISION;
    }
    // limit precision, we don't want to overflow pow10[]
    while (state->precision >= array_len(pow10)) {
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = '0';
        state->precision--;
    }

    int whole = (int) value;
    double tmp = (value - whole) * pow10[state->precision];
    unsigned long frac = (unsigned long) tmp;
    diff = tmp - (double) frac;

    if (diff > 0.5) {
        ++frac;
        // handle rollover, e.g. case 0.99 with precision 1 is 1.0
        if (frac >= pow10[state->precision]) {
            frac = 0;
            ++whole;
        }
    } else if (diff < 0.5) {
    } else if ((frac == 0U) || (frac & 1U)) {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

    if (state->precision == 0U) {
        diff = value - (double) whole;
        if (!((diff < 0.5) || (diff > 0.5)) && (whole & 1)) {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    } else {
        unsigned int count = state->precision;
        // now do fractional part, as an unsigned number
        for (;;) {
            --count;
            if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
                goto ftoa_exceeded;
            buf[len++] = (char) (48U + (frac % 10U));
            if (!(frac /= 10U)) {
                break;
            }
        }
        // add extra 0s
        while (count-- > 0U) {
            if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
                goto ftoa_exceeded;
            buf[len++] = '0';
        }
        // add decimal
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = '.';
    }

    // do whole part, number is reversed
    for (;;) {
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = (char) (48 + (whole % 10));
        if (!(whole /= 10)) {
            break;
        }
    }

    // pad leading zeros
    if (!(state->flags & FMT_FLAG_LEFT) && (state->flags & FMT_FLAG_ZEROPAD)) {
        if (state->width && (negative || (state->flags & (FMT_FLAG_PLUS | FMT_FLAG_SPACE)))) {
            state->width--;
        }
        while (len < state->width) {
            if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
                goto ftoa_exceeded;
            buf[len++] = '0';
        }
    }

    if (negative) {
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = '-';
    } else if (state->flags & FMT_FLAG_PLUS) {
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = '+'; // ignore the space if the '+' exists
    } else if (state->flags & FMT_FLAG_SPACE) {
        if (len == PICO_PRINTF_FTOA_BUFFER_SIZE)
            goto ftoa_exceeded;
        buf[len++] = ' ';
    }

    _out_rev(state, buf, len);
    return;
ftoa_exceeded:
    fmt_state_puts(state, "%!(exceeded PICO_PRINTF_FTOA_BUFFER_SIZE)");
}

#if PICO_PRINTF_SUPPORT_EXPONENTIAL

// internal ftoa variant for exponential floating-point type, contributed by Martijn Jasperse <m.jasperse@gmail.com>
static void _etoa(struct fmt_state *state, double value, bool adapt_exp) {
    // check for NaN and special values
    if (_float_special(state, value))
        return;

    // determine the sign
    const bool negative = value < 0;
    if (negative) {
        value = -value;
    }

    // default precision
    if (!(state->flags & FMT_FLAG_PRECISION)) {
        state->precision = PICO_PRINTF_DEFAULT_FLOAT_PRECISION;
    }

    // determine the decimal exponent
    // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
    union {
        uint64_t U;
        double F;
    } conv;

    conv.F = value;
    int expval;
    if (conv.U) {
        int exp2 = (int) ((conv.U >> 52U) & 0x07FFU) - 1023;         // effectively log2
        conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U); // drop the exponent so conv.F is now in [1,2)
        // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
        expval = (int) (0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
        // now we want to compute 10^expval but we want to be sure it won't overflow
        exp2 = (int) (expval * 3.321928094887362 + 0.5);
        const double z = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
        const double z2 = z * z;
        conv.U = (uint64_t) (exp2 + 1023) << 52U;
        // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
        conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
        // correct for rounding errors
        if (value < conv.F) {
            expval--;
            conv.F /= 10;
        }
    } else {
        expval = 0;
    }

    // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
    unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

    // in "%g" mode, "precision" is the number of *significant figures* not decimals
    if (adapt_exp) {
        // do we want to fall-back to "%f" mode?
        if ((conv.U == 0) || ((value >= 1e-4) && (value < 1e6))) {
            if ((int) state->precision > expval) {
                state->precision = (unsigned) ((int) state->precision - expval - 1);
            } else {
                state->precision = 0;
            }
            state->flags |= FMT_FLAG_PRECISION; // make sure _ftoa respects precision
            // no characters in exponent
            minwidth = 0U;
            expval = 0;
        } else {
            // we use one sigfig for the whole part
            if ((state->precision > 0) && (state->flags & FMT_FLAG_PRECISION)) {
                --state->precision;
            }
        }
    }

    // will everything fit?
    unsigned int fwidth = state->width;
    if (fwidth > minwidth) {
        // we didn't fall-back so subtract the characters required for the exponent
        fwidth -= minwidth;
    } else {
        // not enough characters, so go back to default sizing
        fwidth = 0U;
    }
    if ((state->flags & FMT_FLAG_LEFT) && minwidth) {
        // if we're padding on the right, DON'T pad the floating part
        fwidth = 0U;
    }

    // rescale the float value
    if (expval) {
        value /= conv.F;
    }

    // output the floating part
    const size_t start_idx = fmt_state_len(state);
    struct fmt_state substate = {
        .flags = state->flags,
        .width = fwidth,
        .precision = state->precision,
        .specifier = 'f',
        .ctx = state->ctx,
    };
    _ftoa(&substate, negative ? -value : value);

    // output the exponent part
    if (minwidth) {
        // output the exponential symbol
        fmt_state_putchar(state, _is_upper(state->specifier) ? 'E' : 'e');
        // output the exponent value
        struct fmt_state substate = {
            .flags = FMT_FLAG_ZEROPAD | FMT_FLAG_PLUS,
            .width = minwidth - 1,
            .precision = 0,
            .specifier = 'd',
            .ctx = state->ctx,
        };
        _ntoa(&substate, (unsigned int) ((expval < 0) ? -expval : expval), expval < 0, 10);
        // might need to right-pad spaces
        if (state->flags & FMT_FLAG_LEFT) {
            while (fmt_state_len(state) - start_idx < state->width)
                fmt_state_putchar(state, ' ');
        }
    }
}

#endif // PICO_PRINTF_SUPPORT_EXPONENTIAL
#endif // PICO_PRINTF_SUPPORT_FLOAT

// main ///////////////////////////////////////////////////////////////////////

inline static void _put_quoted_byte(struct fmt_state *state, unsigned char c) {
    fmt_state_putchar(state, '\'');
    if (' ' <= c && c <= '~') {
        if (c == '\'' || c == '\\')
            fmt_state_putchar(state, '\\');
        fmt_state_putchar(state, (char) c);
    } else {
        fmt_state_putchar(state, '\\');
        fmt_state_putchar(state, 'x');
        fmt_state_putchar(state, (c >> 4) & 0xF);
        fmt_state_putchar(state, (c >> 0) & 0xF);
    }
    fmt_state_putchar(state, '\'');
}

static void conv_sint(struct fmt_state *state);
static void conv_uint(struct fmt_state *state);
#if PICO_PRINTF_SUPPORT_FLOAT
static void conv_double(struct fmt_state *state);
#endif
static void conv_char(struct fmt_state *state);
static void conv_str(struct fmt_state *state);
static void conv_ptr(struct fmt_state *state);
static void conv_pct(struct fmt_state *state);

static fmt_specifier_t specifier_table[0x7F] = {
    ['d'] = conv_sint,
    ['i'] = conv_sint,

    ['u'] = conv_uint,
    ['x'] = conv_uint,
    ['X'] = conv_uint,
    ['o'] = conv_uint,
    ['b'] = conv_uint,

#if PICO_PRINTF_SUPPORT_FLOAT
    ['f'] = conv_double,
    ['F'] = conv_double,
#if PICO_PRINTF_SUPPORT_EXPONENTIAL
    ['e'] = conv_double,
    ['E'] = conv_double,
    ['g'] = conv_double,
    ['G'] = conv_double,
#endif
#endif

    ['c'] = conv_char,
    ['s'] = conv_str,
    ['p'] = conv_ptr,
    ['%'] = conv_pct,
};

void fmt_install(char character, fmt_specifier_t fn) {
    unsigned int idx = (unsigned char) character;
    if (idx < array_len(specifier_table) &&
        ' ' < idx && idx <= '~' &&
        !('0' <= idx && idx <= '9'))
        specifier_table[idx] = fn;
}

static void _vfctprintf(struct _fmt_ctx *ctx, const char *format, va_list *va_save) {
    struct fmt_state _state = {
        .args = va_save,
        .ctx = ctx,
    };
    struct fmt_state *state = &_state;

    while (*format) {
        // format specifier?  %[flags][width][.precision][size]specifier
        if (*format != '%') {
            // no
            fmt_state_putchar(state, *format);
            format++;
            continue;
        } else {
            // yes, evaluate it
            format++;
        }

        // evaluate flags
        state->flags = 0U;
        for (;;) {
            switch (*format) {
                case '0':
                    state->flags |= FMT_FLAG_ZEROPAD;
                    format++;
                    break;
                case '-':
                    state->flags |= FMT_FLAG_LEFT;
                    format++;
                    break;
                case '+':
                    state->flags |= FMT_FLAG_PLUS;
                    format++;
                    break;
                case ' ':
                    state->flags |= FMT_FLAG_SPACE;
                    format++;
                    break;
                case '#':
                    state->flags |= FMT_FLAG_HASH;
                    format++;
                    break;
                default:
                    goto no_more_flags;
            }
        }
    no_more_flags:

        // evaluate width field
        state->width = 0U;
        if (_is_digit(*format)) {
            state->width = _atoi(&format);
        } else if (*format == '*') {
            const int w = va_arg(*state->args, int);
            if (w < 0) {
                state->flags |= FMT_FLAG_LEFT; // reverse padding
                state->width = (unsigned int) -w;
            } else {
                state->width = (unsigned int) w;
            }
            format++;
        }

        // evaluate precision field
        state->precision = 0U;
        if (*format == '.') {
            state->flags |= FMT_FLAG_PRECISION;
            format++;
            if (_is_digit(*format)) {
                state->precision = _atoi(&format);
            } else if (*format == '*') {
                const int prec = (int) va_arg(*state->args, int);
                state->precision = prec > 0 ? (unsigned int) prec : 0U;
                format++;
            }
        }

        // evaluate size field
        state->size = FMT_SIZE_DEFAULT;
        switch (*format) {
            case 'l':
                state->size = FMT_SIZE_LONG;
                format++;
                if (*format == 'l') {
                    state->size = FMT_SIZE_LONG_LONG;
                    format++;
                }
                break;
            case 'h':
                state->size = FMT_SIZE_SHORT;
                format++;
                if (*format == 'h') {
                    state->size = FMT_SIZE_CHAR;
                    format++;
                }
                break;
#if PICO_PRINTF_SUPPORT_PTRDIFF_T
            case 't':
                state->size = (sizeof(ptrdiff_t) == sizeof(long) ? FMT_SIZE_LONG : FMT_SIZE_LONG_LONG);
                format++;
                break;
#endif
            case 'j':
                state->size = (sizeof(intmax_t) == sizeof(long) ? FMT_SIZE_LONG : FMT_SIZE_LONG_LONG);
                format++;
                break;
            case 'z':
                state->size = (sizeof(size_t) == sizeof(long) ? FMT_SIZE_LONG : FMT_SIZE_LONG_LONG);
                format++;
                break;
            default:
                break;
        }

        // evaluate specifier
        state->specifier = *format;
        format++;
        if ((unsigned int) state->specifier < array_len(specifier_table) &&
            specifier_table[(unsigned int) state->specifier]) {
            specifier_table[(unsigned int) state->specifier](state);
        } else {
            fmt_state_puts(state, "%!(unknown specifier=");
            _put_quoted_byte(state, (unsigned char) state->specifier);
            fmt_state_putchar(state, ')');
        }
    }
}

int fmt_vfctprintf(fmt_fct_t fct, void *arg, const char *format, va_list _va) {
    struct _fmt_ctx _ctx = {
        .fct = fct,
        .arg = arg,
        .idx = 0,
    };
    va_list _va_save;
    va_copy(_va_save, _va);
    _vfctprintf(&_ctx, format, &_va_save);
    va_end(_va_save);
    return (int) _ctx.idx;
}

void fmt_state_vprintf(struct fmt_state *state, const char *format, va_list _va) {
    va_list _va_save;
    va_copy(_va_save, _va);
    _vfctprintf(state->ctx, format, &_va_save);
    va_end(_va_save);
}

static void conv_sint(struct fmt_state *state) {
    const unsigned int base = 10;
    switch (state->size) {
        case FMT_SIZE_LONG_LONG:
#if PICO_PRINTF_SUPPORT_LONG_LONG
        {
            const long long value = va_arg(*state->args, long long);
            _ntoall(state, (unsigned long long) (value > 0 ? value : 0 - value), value < 0, base);
            break;
        }
#else
            // fall through
#endif
        case FMT_SIZE_LONG: {
            const long value = va_arg(*state->args, long);
            _ntoal(state, (unsigned long) (value > 0 ? value : 0 - value), value < 0, base);
            break;
        }
        case FMT_SIZE_DEFAULT: {
            const int value = va_arg(*state->args, int);
            _ntoa(state, (unsigned int) (value > 0 ? value : 0 - value), value < 0, base);
            break;
        }
        case FMT_SIZE_SHORT: {
            // 'short' is promoted to 'int' when passed through '...'; so we read it
            // with va_arg(*state->args, int), but then truncate it with casting.
            const int value = (short int) va_arg(*state->args, int);
            _ntoa(state, (unsigned int) (value > 0 ? value : 0 - value), value < 0, base);
            break;
        }
        case FMT_SIZE_CHAR: {
            // 'char' is promoted to 'int' when passed through '...'; so we read it
            // with va_arg(*state->args, int), but then truncate it with casting.
            const int value = (char) va_arg(*state->args, int);
            _ntoa(state, (unsigned int) (value > 0 ? value : 0 - value), value < 0, base);
            break;
        }
    }
}

static void conv_uint(struct fmt_state *state) {
    unsigned int base;
    switch (state->specifier) {
        case 'x':
        case 'X':
            base = 16U;
            break;
        case 'o':
            base = 8U;
            break;
        case 'b':
            base = 2U;
            break;
        case 'u':
            base = 10U;
            state->flags &= flipflag(FMT_FLAG_PLUS | FMT_FLAG_SPACE);
            break;
        default:
            __builtin_unreachable();
    }

    switch (state->size) {
        case FMT_SIZE_LONG_LONG:
#if PICO_PRINTF_SUPPORT_LONG_LONG
            _ntoall(state, va_arg(*state->args, unsigned long long), false, base);
            break;
#else
            // fall through
#endif
        case FMT_SIZE_LONG:
            _ntoal(state, va_arg(*state->args, unsigned long), false, base);
            break;
        case FMT_SIZE_DEFAULT:
            _ntoa(state, va_arg(*state->args, unsigned int), false, base);
            break;
        case FMT_SIZE_SHORT:
            // 'short' is promoted to 'int' when passed through '...'; so we read it
            // with va_arg(*state->args, unsigned int), but then truncate it with casting.
            _ntoa(state, (unsigned short int) va_arg(*state->args, unsigned int), false, base);
            break;
        case FMT_SIZE_CHAR:
            // 'char' is promoted to 'int' when passed through '...'; so we read it
            // with va_arg(*state->args, unsigned int), but then truncate it with casting.
            _ntoa(state, (unsigned char) va_arg(*state->args, unsigned int), false, base);
            break;
    }
}

#if PICO_PRINTF_SUPPORT_FLOAT
static void conv_double(struct fmt_state *state) {
    double value = va_arg(*state->args, double);
    switch (state->specifier) {
        case 'f':
        case 'F':
            // test for very large values
            // standard printf behavior is to print EVERY whole number digit -- which could be 100s of characters overflowing your buffers == bad
            if ((value > PICO_PRINTF_MAX_FLOAT && value < DBL_MAX) || (value < -PICO_PRINTF_MAX_FLOAT && value > -DBL_MAX)) {
                fmt_state_puts(state, "%!(exceeded PICO_PRINTF_MAX_FLOAT)");
                return;
            }
            _ftoa(state, value);
            break;
#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        case 'e':
        case 'E':
            _etoa(state, value, false);
            break;
        case 'g':
        case 'G':
            _etoa(state, value, true);
            break;
#endif
    }
}
#endif

static void conv_char(struct fmt_state *state) {
    unsigned int l = 1U;
    // pre padding
    if (!(state->flags & FMT_FLAG_LEFT)) {
        while (l++ < state->width) {
            fmt_state_putchar(state, ' ');
        }
    }
    // char output
    fmt_state_putchar(state, (char) va_arg(*state->args, int));
    // post padding
    if (state->flags & FMT_FLAG_LEFT) {
        while (l++ < state->width) {
            fmt_state_putchar(state, ' ');
        }
    }
}

static void conv_str(struct fmt_state *state) {
    const char *p = va_arg(*state->args, char *);
    unsigned int l = _strnlen_s(p, state->precision ? state->precision : (size_t) -1);
    // pre padding
    if (state->flags & FMT_FLAG_PRECISION) {
        l = (l < state->precision ? l : state->precision);
    }
    if (!(state->flags & FMT_FLAG_LEFT)) {
        while (l++ < state->width) {
            fmt_state_putchar(state, ' ');
        }
    }
    // string output
    while ((*p != 0) && (!(state->flags & FMT_FLAG_PRECISION) || state->precision--)) {
        fmt_state_putchar(state, *(p++));
    }
    // post padding
    if (state->flags & FMT_FLAG_LEFT) {
        while (l++ < state->width) {
            fmt_state_putchar(state, ' ');
        }
    }
}

static void conv_ptr(struct fmt_state *state) {
    state->width = sizeof(void *) * 2U;
    state->flags |= FMT_FLAG_ZEROPAD;
    state->specifier = 'X';

    _Static_assert(sizeof(uintptr_t) == sizeof(int) ||
                   sizeof(uintptr_t) == sizeof(long) ||
                   sizeof(uintptr_t) == sizeof(long long));
    if (sizeof(uintptr_t) == sizeof(int))
        _ntoa(state, (unsigned int) (uintptr_t) va_arg(*state->args, void *), false, 16U);
    else if (sizeof(uintptr_t) == sizeof(long))
        _ntoal(state, (unsigned long) (uintptr_t) va_arg(*state->args, void *), false, 16U);
    else if (sizeof(uintptr_t) == sizeof(long long))
#if PICO_PRINTF_SUPPORT_LONG_LONG
        _ntoall(state, (unsigned long long) (uintptr_t) va_arg(*state->args, void *), false, 16U);
#else
        _ntoal(state, (unsigned long) (uintptr_t) va_arg(*state->args, void *), false, 16U);
#endif
}

static void conv_pct(struct fmt_state *state) {
    fmt_state_putchar(state, '%');
}
