/*
 * Copyright (c) 2025  Luke T. Shumaker
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include <pico/fmt_printf.h>

int main(void) {
    printf("%p\n", fmt_fctprintf);
    printf("%p\n", fmt_vsnprintf);
    printf("%p\n", fmt_snprintf);
    printf("%p\n", fmt_vsprintf);
    printf("%p\n", fmt_sprintf);
    return 0;
}
