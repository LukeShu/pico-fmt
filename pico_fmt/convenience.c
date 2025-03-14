// Copyright (c) 2014-2019  Marco Paland (info@paland.com)
// SPDX-License-Identifier: MIT
//
// Copyright (c) 2020  Raspberry Pi (Trading) Ltd.
// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (C) 2025  Luke T. Shumaker <lukeshu@lukeshu.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pico/fmt_printf.h"

// Outputs /////////////////////////////////////////////////////////////////////

typedef struct {
    char *buffer;
    size_t maxlen;
    size_t cur;
} _arg_buffer;

static void _out_buffer(char character, void *_arg) {
    _arg_buffer *arg = _arg;
    if (arg->cur < arg->maxlen) {
        arg->buffer[arg->cur++] = character;
    }
}

// va_list wrappers ////////////////////////////////////////////////////////////

int fmt_vsnprintf(char *buffer, size_t count, const char *format, va_list va) {
    _arg_buffer arg = {
        .buffer = buffer,
        .maxlen = count,
        .cur = 0,
    };
    const int ret = fmt_vfctprintf(buffer && count ? _out_buffer : NULL, &arg, format, va);
    if (buffer && count)
        buffer[arg.cur < count ? arg.cur : count - 1] = '\0'; // nul-terminate
    return ret;
}

int fmt_vsprintf(char *buffer, const char *format, va_list va) {
    return fmt_vsnprintf(buffer, (size_t) -1, format, va);
}

// Var-args wrappers ///////////////////////////////////////////////////////////

int fmt_fctprintf(fmt_fct_t out, void *arg, const char *format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = fmt_vfctprintf(out, arg, format, va);
    va_end(va);
    return ret;
}

int fmt_snprintf(char *buffer, size_t count, const char *format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = fmt_vsnprintf(buffer, count, format, va);
    va_end(va);
    return ret;
}

int fmt_sprintf(char *buffer, const char *format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = fmt_vsprintf(buffer, format, va);
    va_end(va);
    return ret;
}
