/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico.h"
#include "pico/fmt_printf.h"
#include "pico/printf.h"

#if !PICO_PRINTF_ALWAYS_INCLUDED
// we don't have a way to specify a truly weak symbol reference (the linker will always include targets in a single link step,
// so we make a function pointer that is initialized on the first printf called... if printf is not included in the binary
// (or has never been called - we can't tell) then this will be null. the assumption is that if you are using printf
// you are likely to have printed something.
static int (*lazy_vfctprintf)(void (*out)(char character, void *arg), void *arg, const char *format, va_list va);
#endif

// output: [F]un[CT]ion //////////////////////////////////////////////

int vfctprintf(void (*out)(char character, void *arg), void *arg, const char *format, va_list va) {
#if !PICO_PRINTF_ALWAYS_INCLUDED
    lazy_vfctprintf = fmt_vfctprintf;
#endif
    return fmt_vfctprintf(out, arg, format, va);
}

// output: [S]tring //////////////////////////////////////////////////

int WRAPPER_FUNC(vsnprintf)(char *buffer, size_t count, const char *format, va_list va)
{
#if !PICO_PRINTF_ALWAYS_INCLUDED
    lazy_vfctprintf = fmt_vfctprintf;
#endif
    return fmt_vsnprintf(buffer, count, format, va);
}

int WRAPPER_FUNC(snprintf)(char *buffer, size_t count, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = WRAPPER_FUNC(vsnprintf)(buffer, count, format, va);
    va_end(va);
    return ret;
}
int WRAPPER_FUNC(sprintf)(char *buffer, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = WRAPPER_FUNC(vsnprintf)(buffer, (size_t) -1, format, va);
    va_end(va);
    return ret;
}

// output: stdout ////////////////////////////////////////////////////
//
// The main stdout-printf functions are in pico_stdio; these are
// merely stubs for use by pico_stdio.

#if !PICO_PRINTF_ALWAYS_INCLUDED

/**
 * Output a character to a custom device like UART, used by the printf() function
 * This function is declared here only. You have to write your custom implementation somewhere
 * \param character Character to output
 */
static void _out_char(char character, void *arg) {
    (void) arg;
    if (character) {
        putchar(character);
    }
}

bool weak_raw_printf(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    bool rc = weak_raw_vprintf(fmt, va);
    va_end(va);
    return rc;
}

bool weak_raw_vprintf(const char *fmt, va_list args) {
    if (lazy_vfctprintf) {
        lazy_vfctprintf(_out_char, NULL, fmt, args);
        return true;
    } else {
        puts(fmt);
        return false;
    }
}

#endif
