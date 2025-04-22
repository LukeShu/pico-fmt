// Copyright (c) 2017-2019  Marco Paland (info@paland.com)
// SPDX-License-Identifier: MIT
//
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
// SPDX-License-Identifier: BSD-3-Clause
//
// Copyright (c) 2025  Luke T. Shumaker
// SPDX-License-Identifier: BSD-3-Clause
//
///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2017-2019, PALANDesign Hannover, Germany
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
// \brief printf unit tests
//
///////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pico/fmt_printf.h"

static char printf_buffer[100];
static size_t printf_idx = 0U;

void _out_fct(char character, void *arg) {
    (void) arg;
    printf_buffer[printf_idx++] = character;
}

int fmt_vprintf(const char *format, va_list va) [[gnu::format(printf, 1, 0)]] {
    return fmt_vfctprintf(_out_fct, NULL, format, va);
}

int fmt_printf(const char *format, ...) [[gnu::format(printf, 1, 2)]] {
    va_list va;
    va_start(va, format);
    int ret = fmt_vfctprintf(_out_fct, NULL, format, va);
    va_end(va);
    return ret;
}

#define TEST_CASE(GRP_NAME, ...)                                                                                     \
    do {                                                                                                             \
        grp_name = GRP_NAME;                                                                                         \
        printf("%.70s\n", "== " GRP_NAME " ======================================================================"); \
    } while (0)
#define REQUIRE(expr) _REQUIRE(expr, #expr)
#define _REQUIRE(expr, expr_str)                                                                       \
    do {                                                                                               \
        if (!(expr)) {                                                                                 \
            printf("failure: %s:%u:%s: REQUIRE failed: %s\n", __FILE__, __LINE__, grp_name, expr_str); \
            failures++;                                                                                \
        }                                                                                              \
    } while (0)
#define REQUIRE_STREQ(act, exp)                                 \
    do {                                                        \
        if (strcmp(act, exp)) {                                 \
            printf("failure: %s:%u:%s: REQUIRE_STREQ failed:\n" \
                   "\tactual  : \"%s\"\n"                       \
                   "\texpected: \"%s\"\n",                      \
                   __FILE__, __LINE__, grp_name,                \
                   act, exp);                                   \
            failures++;                                         \
        }                                                       \
    } while (0)

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"

static void vprintf_builder_1(char *buffer, ...) {
    va_list args;
    va_start(args, buffer);
    fmt_vprintf("%d", args);
    va_end(args);
}

static void vsnprintf_builder_1(char *buffer, ...) {
    va_list args;
    va_start(args, buffer);
    fmt_vsnprintf(buffer, 100U, "%d", args);
    va_end(args);
}

static void vsnprintf_builder_3(char *buffer, ...) {
    va_list args;
    va_start(args, buffer);
    fmt_vsnprintf(buffer, 100U, "%d %d %s", args);
    va_end(args);
}

int main(void) {
    const char *grp_name;
    unsigned int failures = 0;

    TEST_CASE("printf", "[]");
    {
        printf_idx = 0U;
        memset(printf_buffer, 0xCC, 100U);
        REQUIRE(fmt_printf("% d", 4232) == 5);
        REQUIRE(printf_buffer[5] == (char) 0xCC);
        printf_buffer[5] = 0;
        REQUIRE_STREQ(printf_buffer, " 4232");
    }

    TEST_CASE("fctprintf", "[]");
    {
        printf_idx = 0U;
        memset(printf_buffer, 0xCC, 100U);
        fmt_fctprintf(&_out_fct, NULL, "This is a test of %X", 0x12EFU);
        REQUIRE(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
        REQUIRE(printf_buffer[22] == (char) 0xCC);
    }

    TEST_CASE("snprintf", "[]");
    {
        char buffer[100];

        fmt_snprintf(buffer, 100U, "%d", -1000);
        REQUIRE_STREQ(buffer, "-1000");

        fmt_snprintf(buffer, 3U, "%d", -1000);
        REQUIRE_STREQ(buffer, "-1");
    }

    TEST_CASE("vprintf", "[]");
    {
        char buffer[100];
        printf_idx = 0U;
        memset(printf_buffer, 0xCC, 100U);
        vprintf_builder_1(buffer, 2345);
        REQUIRE(printf_buffer[4] == (char) 0xCC);
        printf_buffer[4] = 0;
        REQUIRE_STREQ(printf_buffer, "2345");
    }

    TEST_CASE("vsnprintf", "[]");
    {
        char buffer[100];

        vsnprintf_builder_1(buffer, -1);
        REQUIRE_STREQ(buffer, "-1");

        vsnprintf_builder_3(buffer, 3, -1000, "test");
        REQUIRE_STREQ(buffer, "3 -1000 test");
    }

    TEST_CASE("space flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "% d", 42);
        REQUIRE_STREQ(buffer, " 42");

        fmt_sprintf(buffer, "% d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "% 5d", 42);
        REQUIRE_STREQ(buffer, "   42");

        fmt_sprintf(buffer, "% 5d", -42);
        REQUIRE_STREQ(buffer, "  -42");

        fmt_sprintf(buffer, "% 15d", 42);
        REQUIRE_STREQ(buffer, "             42");

        fmt_sprintf(buffer, "% 15d", -42);
        REQUIRE_STREQ(buffer, "            -42");

        fmt_sprintf(buffer, "% 15d", -42);
        REQUIRE_STREQ(buffer, "            -42");

#if PICO_PRINTF_SUPPORT_FLOAT
        fmt_sprintf(buffer, "% 15.3f", -42.987);
        REQUIRE_STREQ(buffer, "        -42.987");

        fmt_sprintf(buffer, "% 15.3f", 42.987);
        REQUIRE_STREQ(buffer, "         42.987");
#endif

        fmt_sprintf(buffer, "% s", "Hello testing");
        REQUIRE_STREQ(buffer, "Hello testing");

        fmt_sprintf(buffer, "% d", 1024);
        REQUIRE_STREQ(buffer, " 1024");

        fmt_sprintf(buffer, "% d", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "% i", 1024);
        REQUIRE_STREQ(buffer, " 1024");

        fmt_sprintf(buffer, "% i", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "% u", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "% u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272");

        fmt_sprintf(buffer, "% o", 511);
        REQUIRE_STREQ(buffer, "777");

        fmt_sprintf(buffer, "% o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001");

        fmt_sprintf(buffer, "% x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd");

        fmt_sprintf(buffer, "% x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433");

        fmt_sprintf(buffer, "% X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD");

        fmt_sprintf(buffer, "% X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433");

        fmt_sprintf(buffer, "% c", 'x');
        REQUIRE_STREQ(buffer, "x");
    }

    TEST_CASE("+ flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%+d", 42);
        REQUIRE_STREQ(buffer, "+42");

        fmt_sprintf(buffer, "%+d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "%+5d", 42);
        REQUIRE_STREQ(buffer, "  +42");

        fmt_sprintf(buffer, "%+5d", -42);
        REQUIRE_STREQ(buffer, "  -42");

        fmt_sprintf(buffer, "%+15d", 42);
        REQUIRE_STREQ(buffer, "            +42");

        fmt_sprintf(buffer, "%+15d", -42);
        REQUIRE_STREQ(buffer, "            -42");

        fmt_sprintf(buffer, "%+s", "Hello testing");
        REQUIRE_STREQ(buffer, "Hello testing");

        fmt_sprintf(buffer, "%+d", 1024);
        REQUIRE_STREQ(buffer, "+1024");

        fmt_sprintf(buffer, "%+d", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%+i", 1024);
        REQUIRE_STREQ(buffer, "+1024");

        fmt_sprintf(buffer, "%+i", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%+u", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%+u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272");

        fmt_sprintf(buffer, "%+o", 511);
        REQUIRE_STREQ(buffer, "777");

        fmt_sprintf(buffer, "%+o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001");

        fmt_sprintf(buffer, "%+x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd");

        fmt_sprintf(buffer, "%+x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433");

        fmt_sprintf(buffer, "%+X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD");

        fmt_sprintf(buffer, "%+X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433");

        fmt_sprintf(buffer, "%+c", 'x');
        REQUIRE_STREQ(buffer, "x");

        fmt_sprintf(buffer, "%+.0d", 0);
        REQUIRE_STREQ(buffer, "+");
    }

    TEST_CASE("0 flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%0d", 42);
        REQUIRE_STREQ(buffer, "42");

        fmt_sprintf(buffer, "%0ld", 42L);
        REQUIRE_STREQ(buffer, "42");

        fmt_sprintf(buffer, "%0d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "%05d", 42);
        REQUIRE_STREQ(buffer, "00042");

        fmt_sprintf(buffer, "%05d", -42);
        REQUIRE_STREQ(buffer, "-0042");

        fmt_sprintf(buffer, "%015d", 42);
        REQUIRE_STREQ(buffer, "000000000000042");

        fmt_sprintf(buffer, "%015d", -42);
        REQUIRE_STREQ(buffer, "-00000000000042");

#if PICO_PRINTF_SUPPORT_FLOAT
        fmt_sprintf(buffer, "%015.2f", 42.1234);
        REQUIRE_STREQ(buffer, "000000000042.12");

        fmt_sprintf(buffer, "%015.3f", 42.9876);
        REQUIRE_STREQ(buffer, "00000000042.988");

        fmt_sprintf(buffer, "%015.5f", -42.9876);
        REQUIRE_STREQ(buffer, "-00000042.98760");
#endif
    }

    TEST_CASE("- flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%-d", 42);
        REQUIRE_STREQ(buffer, "42");

        fmt_sprintf(buffer, "%-d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "%-5d", 42);
        REQUIRE_STREQ(buffer, "42   ");

        fmt_sprintf(buffer, "%-5d", -42);
        REQUIRE_STREQ(buffer, "-42  ");

        fmt_sprintf(buffer, "%-15d", 42);
        REQUIRE_STREQ(buffer, "42             ");

        fmt_sprintf(buffer, "%-15d", -42);
        REQUIRE_STREQ(buffer, "-42            ");

        fmt_sprintf(buffer, "%-0d", 42);
        REQUIRE_STREQ(buffer, "42");

        fmt_sprintf(buffer, "%-0d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "%-05d", 42);
        REQUIRE_STREQ(buffer, "42   ");

        fmt_sprintf(buffer, "%-05d", -42);
        REQUIRE_STREQ(buffer, "-42  ");

        fmt_sprintf(buffer, "%-015d", 42);
        REQUIRE_STREQ(buffer, "42             ");

        fmt_sprintf(buffer, "%-015d", -42);
        REQUIRE_STREQ(buffer, "-42            ");

        fmt_sprintf(buffer, "%0-d", 42);
        REQUIRE_STREQ(buffer, "42");

        fmt_sprintf(buffer, "%0-d", -42);
        REQUIRE_STREQ(buffer, "-42");

        fmt_sprintf(buffer, "%0-5d", 42);
        REQUIRE_STREQ(buffer, "42   ");

        fmt_sprintf(buffer, "%0-5d", -42);
        REQUIRE_STREQ(buffer, "-42  ");

        fmt_sprintf(buffer, "%0-15d", 42);
        REQUIRE_STREQ(buffer, "42             ");

        fmt_sprintf(buffer, "%0-15d", -42);
        REQUIRE_STREQ(buffer, "-42            ");

        fmt_sprintf(buffer, "%0-15.3e", -42.);
#if PICO_PRINTF_SUPPORT_FLOAT && PICO_PRINTF_SUPPORT_EXPONENTIAL
        REQUIRE_STREQ(buffer, "-4.200e+01     ");
#else
        REQUIRE_STREQ(buffer, "%!(unknown specifier='e')");
#endif

        fmt_sprintf(buffer, "%0-15.3g", -42.);
#if PICO_PRINTF_SUPPORT_FLOAT && PICO_PRINTF_SUPPORT_EXPONENTIAL
        REQUIRE_STREQ(buffer, "-42.0          ");
#else
        REQUIRE_STREQ(buffer, "%!(unknown specifier='g')");
#endif
    }

    TEST_CASE("# flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%#.0x", 0);
        REQUIRE_STREQ(buffer, "");
        fmt_sprintf(buffer, "%#.1x", 0);
        REQUIRE_STREQ(buffer, "0");
        fmt_sprintf(buffer, "%#.0llx", (long long) 0);
        REQUIRE_STREQ(buffer, "");
        fmt_sprintf(buffer, "%#.8x", 0x614e);
        REQUIRE_STREQ(buffer, "0x0000614e");
        fmt_sprintf(buffer, "%#b", 6);
        REQUIRE_STREQ(buffer, "0b110");
    }

    TEST_CASE("specifier", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "Hello testing");
        REQUIRE_STREQ(buffer, "Hello testing");

        fmt_sprintf(buffer, "%s", "Hello testing");
        REQUIRE_STREQ(buffer, "Hello testing");

        fmt_sprintf(buffer, "%d", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%d", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%i", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%i", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%u", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272");

        fmt_sprintf(buffer, "%o", 511);
        REQUIRE_STREQ(buffer, "777");

        fmt_sprintf(buffer, "%o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001");

        fmt_sprintf(buffer, "%x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd");

        fmt_sprintf(buffer, "%x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433");

        fmt_sprintf(buffer, "%X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD");

        fmt_sprintf(buffer, "%X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433");

        fmt_sprintf(buffer, "%%");
        REQUIRE_STREQ(buffer, "%");
    }

    TEST_CASE("width", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%1s", "Hello testing");
        REQUIRE_STREQ(buffer, "Hello testing");

        fmt_sprintf(buffer, "%1d", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%1d", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%1i", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%1i", -1024);
        REQUIRE_STREQ(buffer, "-1024");

        fmt_sprintf(buffer, "%1u", 1024);
        REQUIRE_STREQ(buffer, "1024");

        fmt_sprintf(buffer, "%1u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272");

        fmt_sprintf(buffer, "%1o", 511);
        REQUIRE_STREQ(buffer, "777");

        fmt_sprintf(buffer, "%1o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001");

        fmt_sprintf(buffer, "%1x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd");

        fmt_sprintf(buffer, "%1x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433");

        fmt_sprintf(buffer, "%1X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD");

        fmt_sprintf(buffer, "%1X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433");

        fmt_sprintf(buffer, "%1c", 'x');
        REQUIRE_STREQ(buffer, "x");
    }

    TEST_CASE("width 20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%20s", "Hello");
        REQUIRE_STREQ(buffer, "               Hello");

        fmt_sprintf(buffer, "%20d", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20d", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%20i", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20i", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%20u", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20u", 4294966272U);
        REQUIRE_STREQ(buffer, "          4294966272");

        fmt_sprintf(buffer, "%20o", 511);
        REQUIRE_STREQ(buffer, "                 777");

        fmt_sprintf(buffer, "%20o", 4294966785U);
        REQUIRE_STREQ(buffer, "         37777777001");

        fmt_sprintf(buffer, "%20x", 305441741);
        REQUIRE_STREQ(buffer, "            1234abcd");

        fmt_sprintf(buffer, "%20x", 3989525555U);
        REQUIRE_STREQ(buffer, "            edcb5433");

        fmt_sprintf(buffer, "%20X", 305441741);
        REQUIRE_STREQ(buffer, "            1234ABCD");

        fmt_sprintf(buffer, "%20X", 3989525555U);
        REQUIRE_STREQ(buffer, "            EDCB5433");

        fmt_sprintf(buffer, "%20c", 'x');
        REQUIRE_STREQ(buffer, "                   x");
    }

    TEST_CASE("width *20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%*s", 20, "Hello");
        REQUIRE_STREQ(buffer, "               Hello");

        fmt_sprintf(buffer, "%*d", 20, 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%*d", 20, -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%*i", 20, 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%*i", 20, -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%*u", 20, 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%*u", 20, 4294966272U);
        REQUIRE_STREQ(buffer, "          4294966272");

        fmt_sprintf(buffer, "%*o", 20, 511);
        REQUIRE_STREQ(buffer, "                 777");

        fmt_sprintf(buffer, "%*o", 20, 4294966785U);
        REQUIRE_STREQ(buffer, "         37777777001");

        fmt_sprintf(buffer, "%*x", 20, 305441741);
        REQUIRE_STREQ(buffer, "            1234abcd");

        fmt_sprintf(buffer, "%*x", 20, 3989525555U);
        REQUIRE_STREQ(buffer, "            edcb5433");

        fmt_sprintf(buffer, "%*X", 20, 305441741);
        REQUIRE_STREQ(buffer, "            1234ABCD");

        fmt_sprintf(buffer, "%*X", 20, 3989525555U);
        REQUIRE_STREQ(buffer, "            EDCB5433");

        fmt_sprintf(buffer, "%*c", 20, 'x');
        REQUIRE_STREQ(buffer, "                   x");
    }

    TEST_CASE("width -20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%-20s", "Hello");
        REQUIRE_STREQ(buffer, "Hello               ");

        fmt_sprintf(buffer, "%-20d", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

        fmt_sprintf(buffer, "%-20d", -1024);
        REQUIRE_STREQ(buffer, "-1024               ");

        fmt_sprintf(buffer, "%-20i", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

        fmt_sprintf(buffer, "%-20i", -1024);
        REQUIRE_STREQ(buffer, "-1024               ");

        fmt_sprintf(buffer, "%-20u", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

#if PICO_PRINTF_SUPPORT_FLOAT
        fmt_sprintf(buffer, "%-20.4f", 1024.1234);
        REQUIRE_STREQ(buffer, "1024.1234           ");
#endif

        fmt_sprintf(buffer, "%-20u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272          ");

        fmt_sprintf(buffer, "%-20o", 511);
        REQUIRE_STREQ(buffer, "777                 ");

        fmt_sprintf(buffer, "%-20o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001         ");

        fmt_sprintf(buffer, "%-20x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd            ");

        fmt_sprintf(buffer, "%-20x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433            ");

        fmt_sprintf(buffer, "%-20X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD            ");

        fmt_sprintf(buffer, "%-20X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433            ");

        fmt_sprintf(buffer, "%-20c", 'x');
        REQUIRE_STREQ(buffer, "x                   ");

        fmt_sprintf(buffer, "|%5d| |%-2d| |%5d|", 9, 9, 9);
        REQUIRE_STREQ(buffer, "|    9| |9 | |    9|");

        fmt_sprintf(buffer, "|%5d| |%-2d| |%5d|", 10, 10, 10);
        REQUIRE_STREQ(buffer, "|   10| |10| |   10|");

        fmt_sprintf(buffer, "|%5d| |%-12d| |%5d|", 9, 9, 9);
        REQUIRE_STREQ(buffer, "|    9| |9           | |    9|");

        fmt_sprintf(buffer, "|%5d| |%-12d| |%5d|", 10, 10, 10);
        REQUIRE_STREQ(buffer, "|   10| |10          | |   10|");
    }

    TEST_CASE("width 0-20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%0-20s", "Hello");
        REQUIRE_STREQ(buffer, "Hello               ");

        fmt_sprintf(buffer, "%0-20d", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

        fmt_sprintf(buffer, "%0-20d", -1024);
        REQUIRE_STREQ(buffer, "-1024               ");

        fmt_sprintf(buffer, "%0-20i", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

        fmt_sprintf(buffer, "%0-20i", -1024);
        REQUIRE_STREQ(buffer, "-1024               ");

        fmt_sprintf(buffer, "%0-20u", 1024);
        REQUIRE_STREQ(buffer, "1024                ");

        fmt_sprintf(buffer, "%0-20u", 4294966272U);
        REQUIRE_STREQ(buffer, "4294966272          ");

        fmt_sprintf(buffer, "%0-20o", 511);
        REQUIRE_STREQ(buffer, "777                 ");

        fmt_sprintf(buffer, "%0-20o", 4294966785U);
        REQUIRE_STREQ(buffer, "37777777001         ");

        fmt_sprintf(buffer, "%0-20x", 305441741);
        REQUIRE_STREQ(buffer, "1234abcd            ");

        fmt_sprintf(buffer, "%0-20x", 3989525555U);
        REQUIRE_STREQ(buffer, "edcb5433            ");

        fmt_sprintf(buffer, "%0-20X", 305441741);
        REQUIRE_STREQ(buffer, "1234ABCD            ");

        fmt_sprintf(buffer, "%0-20X", 3989525555U);
        REQUIRE_STREQ(buffer, "EDCB5433            ");

        fmt_sprintf(buffer, "%0-20c", 'x');
        REQUIRE_STREQ(buffer, "x                   ");
    }

    TEST_CASE("padding 20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%020d", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%020d", -1024);
        REQUIRE_STREQ(buffer, "-0000000000000001024");

        fmt_sprintf(buffer, "%020i", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%020i", -1024);
        REQUIRE_STREQ(buffer, "-0000000000000001024");

        fmt_sprintf(buffer, "%020u", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%020u", 4294966272U);
        REQUIRE_STREQ(buffer, "00000000004294966272");

        fmt_sprintf(buffer, "%020o", 511);
        REQUIRE_STREQ(buffer, "00000000000000000777");

        fmt_sprintf(buffer, "%020o", 4294966785U);
        REQUIRE_STREQ(buffer, "00000000037777777001");

        fmt_sprintf(buffer, "%020x", 305441741);
        REQUIRE_STREQ(buffer, "0000000000001234abcd");

        fmt_sprintf(buffer, "%020x", 3989525555U);
        REQUIRE_STREQ(buffer, "000000000000edcb5433");

        fmt_sprintf(buffer, "%020X", 305441741);
        REQUIRE_STREQ(buffer, "0000000000001234ABCD");

        fmt_sprintf(buffer, "%020X", 3989525555U);
        REQUIRE_STREQ(buffer, "000000000000EDCB5433");
    }

    TEST_CASE("padding .20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%.20d", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%.20d", -1024);
        REQUIRE_STREQ(buffer, "-00000000000000001024");

        fmt_sprintf(buffer, "%.20i", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%.20i", -1024);
        REQUIRE_STREQ(buffer, "-00000000000000001024");

        fmt_sprintf(buffer, "%.20u", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%.20u", 4294966272U);
        REQUIRE_STREQ(buffer, "00000000004294966272");

        fmt_sprintf(buffer, "%.20o", 511);
        REQUIRE_STREQ(buffer, "00000000000000000777");

        fmt_sprintf(buffer, "%.20o", 4294966785U);
        REQUIRE_STREQ(buffer, "00000000037777777001");

        fmt_sprintf(buffer, "%.20x", 305441741);
        REQUIRE_STREQ(buffer, "0000000000001234abcd");

        fmt_sprintf(buffer, "%.20x", 3989525555U);
        REQUIRE_STREQ(buffer, "000000000000edcb5433");

        fmt_sprintf(buffer, "%.20X", 305441741);
        REQUIRE_STREQ(buffer, "0000000000001234ABCD");

        fmt_sprintf(buffer, "%.20X", 3989525555U);
        REQUIRE_STREQ(buffer, "000000000000EDCB5433");
    }

    TEST_CASE("padding #020", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%#020d", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%#020d", -1024);
        REQUIRE_STREQ(buffer, "-0000000000000001024");

        fmt_sprintf(buffer, "%#020i", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%#020i", -1024);
        REQUIRE_STREQ(buffer, "-0000000000000001024");

        fmt_sprintf(buffer, "%#020u", 1024);
        REQUIRE_STREQ(buffer, "00000000000000001024");

        fmt_sprintf(buffer, "%#020u", 4294966272U);
        REQUIRE_STREQ(buffer, "00000000004294966272");

        fmt_sprintf(buffer, "%#020o", 511);
        REQUIRE_STREQ(buffer, "00000000000000000777");

        fmt_sprintf(buffer, "%#020o", 4294966785U);
        REQUIRE_STREQ(buffer, "00000000037777777001");

        fmt_sprintf(buffer, "%#020x", 305441741);
        REQUIRE_STREQ(buffer, "0x00000000001234abcd");

        fmt_sprintf(buffer, "%#020x", 3989525555U);
        REQUIRE_STREQ(buffer, "0x0000000000edcb5433");

        fmt_sprintf(buffer, "%#020X", 305441741);
        REQUIRE_STREQ(buffer, "0X00000000001234ABCD");

        fmt_sprintf(buffer, "%#020X", 3989525555U);
        REQUIRE_STREQ(buffer, "0X0000000000EDCB5433");
    }

    TEST_CASE("padding #20", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%#20d", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%#20d", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%#20i", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%#20i", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%#20u", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%#20u", 4294966272U);
        REQUIRE_STREQ(buffer, "          4294966272");

        fmt_sprintf(buffer, "%#20o", 511);
        REQUIRE_STREQ(buffer, "                0777");

        fmt_sprintf(buffer, "%#20o", 4294966785U);
        REQUIRE_STREQ(buffer, "        037777777001");

        fmt_sprintf(buffer, "%#20x", 305441741);
        REQUIRE_STREQ(buffer, "          0x1234abcd");

        fmt_sprintf(buffer, "%#20x", 3989525555U);
        REQUIRE_STREQ(buffer, "          0xedcb5433");

        fmt_sprintf(buffer, "%#20X", 305441741);
        REQUIRE_STREQ(buffer, "          0X1234ABCD");

        fmt_sprintf(buffer, "%#20X", 3989525555U);
        REQUIRE_STREQ(buffer, "          0XEDCB5433");
    }

    TEST_CASE("padding 20.5", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%20.5d", 1024);
        REQUIRE_STREQ(buffer, "               01024");

        fmt_sprintf(buffer, "%20.5d", -1024);
        REQUIRE_STREQ(buffer, "              -01024");

        fmt_sprintf(buffer, "%20.5i", 1024);
        REQUIRE_STREQ(buffer, "               01024");

        fmt_sprintf(buffer, "%20.5i", -1024);
        REQUIRE_STREQ(buffer, "              -01024");

        fmt_sprintf(buffer, "%20.5u", 1024);
        REQUIRE_STREQ(buffer, "               01024");

        fmt_sprintf(buffer, "%20.5u", 4294966272U);
        REQUIRE_STREQ(buffer, "          4294966272");

        fmt_sprintf(buffer, "%20.5o", 511);
        REQUIRE_STREQ(buffer, "               00777");

        fmt_sprintf(buffer, "%20.5o", 4294966785U);
        REQUIRE_STREQ(buffer, "         37777777001");

        fmt_sprintf(buffer, "%20.5x", 305441741);
        REQUIRE_STREQ(buffer, "            1234abcd");

        fmt_sprintf(buffer, "%20.10x", 3989525555U);
        REQUIRE_STREQ(buffer, "          00edcb5433");

        fmt_sprintf(buffer, "%20.5X", 305441741);
        REQUIRE_STREQ(buffer, "            1234ABCD");

        fmt_sprintf(buffer, "%20.10X", 3989525555U);
        REQUIRE_STREQ(buffer, "          00EDCB5433");
    }

    TEST_CASE("padding neg numbers", "[]");
    {
        char buffer[100];

        // space padding
        fmt_sprintf(buffer, "% 1d", -5);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "% 2d", -5);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "% 3d", -5);
        REQUIRE_STREQ(buffer, " -5");

        fmt_sprintf(buffer, "% 4d", -5);
        REQUIRE_STREQ(buffer, "  -5");

        // zero padding
        fmt_sprintf(buffer, "%01d", -5);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "%02d", -5);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "%03d", -5);
        REQUIRE_STREQ(buffer, "-05");

        fmt_sprintf(buffer, "%04d", -5);
        REQUIRE_STREQ(buffer, "-005");
    }

#if PICO_PRINTF_SUPPORT_FLOAT
    TEST_CASE("float padding neg numbers", "[]");
    {
        char buffer[100];

        // space padding
        fmt_sprintf(buffer, "% 3.1f", -5.);
        REQUIRE_STREQ(buffer, "-5.0");

        fmt_sprintf(buffer, "% 4.1f", -5.);
        REQUIRE_STREQ(buffer, "-5.0");

        fmt_sprintf(buffer, "% 5.1f", -5.);
        REQUIRE_STREQ(buffer, " -5.0");

#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        fmt_sprintf(buffer, "% 6.1g", -5.);
        REQUIRE_STREQ(buffer, "    -5");

        fmt_sprintf(buffer, "% 6.1e", -5.);
        REQUIRE_STREQ(buffer, "-5.0e+00");

        fmt_sprintf(buffer, "% 10.1e", -5.);
        REQUIRE_STREQ(buffer, "  -5.0e+00");
#endif

        // zero padding
        fmt_sprintf(buffer, "%03.1f", -5.);
        REQUIRE_STREQ(buffer, "-5.0");

        fmt_sprintf(buffer, "%04.1f", -5.);
        REQUIRE_STREQ(buffer, "-5.0");

        fmt_sprintf(buffer, "%05.1f", -5.);
        REQUIRE_STREQ(buffer, "-05.0");

        // zero padding no decimal point
        fmt_sprintf(buffer, "%01.0f", -5.);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "%02.0f", -5.);
        REQUIRE_STREQ(buffer, "-5");

        fmt_sprintf(buffer, "%03.0f", -5.);
        REQUIRE_STREQ(buffer, "-05");

#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        fmt_sprintf(buffer, "%010.1e", -5.);
        REQUIRE_STREQ(buffer, "-005.0e+00");

        fmt_sprintf(buffer, "%07.0E", -5.);
        REQUIRE_STREQ(buffer, "-05E+00");

        fmt_sprintf(buffer, "%03.0g", -5.);
        REQUIRE_STREQ(buffer, "-05");
#endif
    }
#endif

    TEST_CASE("length", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%.0s", "Hello testing");
        REQUIRE_STREQ(buffer, "");

        fmt_sprintf(buffer, "%20.0s", "Hello testing");
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%.s", "Hello testing");
        REQUIRE_STREQ(buffer, "");

        fmt_sprintf(buffer, "%20.s", "Hello testing");
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.0d", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20.0d", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%20.d", 0);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.0i", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20.i", -1024);
        REQUIRE_STREQ(buffer, "               -1024");

        fmt_sprintf(buffer, "%20.i", 0);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.u", 1024);
        REQUIRE_STREQ(buffer, "                1024");

        fmt_sprintf(buffer, "%20.0u", 4294966272U);
        REQUIRE_STREQ(buffer, "          4294966272");

        fmt_sprintf(buffer, "%20.u", 0U);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.o", 511);
        REQUIRE_STREQ(buffer, "                 777");

        fmt_sprintf(buffer, "%20.0o", 4294966785U);
        REQUIRE_STREQ(buffer, "         37777777001");

        fmt_sprintf(buffer, "%20.o", 0U);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.x", 305441741);
        REQUIRE_STREQ(buffer, "            1234abcd");

        fmt_sprintf(buffer, "%50.x", 305441741);
        REQUIRE_STREQ(buffer, "                                          1234abcd");

        fmt_sprintf(buffer, "%50.x%10.u", 305441741, 12345);
        REQUIRE_STREQ(buffer, "                                          1234abcd     12345");

        fmt_sprintf(buffer, "%20.0x", 3989525555U);
        REQUIRE_STREQ(buffer, "            edcb5433");

        fmt_sprintf(buffer, "%20.x", 0U);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%20.X", 305441741);
        REQUIRE_STREQ(buffer, "            1234ABCD");

        fmt_sprintf(buffer, "%20.0X", 3989525555U);
        REQUIRE_STREQ(buffer, "            EDCB5433");

        fmt_sprintf(buffer, "%20.X", 0U);
        REQUIRE_STREQ(buffer, "                    ");

        fmt_sprintf(buffer, "%02.0u", 0U);
        REQUIRE_STREQ(buffer, "  ");

        fmt_sprintf(buffer, "%02.0d", 0);
        REQUIRE_STREQ(buffer, "  ");
    }

#if PICO_PRINTF_SUPPORT_FLOAT
    TEST_CASE("float", "[]");
    {
        char buffer[100];
        char libc_buffer[100];

        // test special-case floats using math.h macros
        fmt_sprintf(buffer, "%8f", NAN);
        REQUIRE_STREQ(buffer, "     nan");

        fmt_sprintf(buffer, "%8f", INFINITY);
        REQUIRE_STREQ(buffer, "     inf");

        fmt_sprintf(buffer, "%-8f", -INFINITY);
        REQUIRE_STREQ(buffer, "-inf    ");

#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        fmt_sprintf(buffer, "%+8e", INFINITY);
        REQUIRE_STREQ(buffer, "    +inf");
#endif

        fmt_sprintf(buffer, "%.4f", 3.1415354);
        REQUIRE_STREQ(buffer, "3.1415");

        fmt_sprintf(buffer, "%.3f", 30343.1415354);
        REQUIRE_STREQ(buffer, "30343.142");

        fmt_sprintf(buffer, "%.0f", 34.1415354);
        REQUIRE_STREQ(buffer, "34");

        fmt_sprintf(buffer, "%.0f", 1.3);
        REQUIRE_STREQ(buffer, "1");

        fmt_sprintf(buffer, "%.0f", 1.55);
        REQUIRE_STREQ(buffer, "2");

        fmt_sprintf(buffer, "%.1f", 1.64);
        REQUIRE_STREQ(buffer, "1.6");

        fmt_sprintf(buffer, "%.2f", 42.8952);
        REQUIRE_STREQ(buffer, "42.90");

        fmt_sprintf(buffer, "%.9f", 42.8952);
        REQUIRE_STREQ(buffer, "42.895200000");

        fmt_sprintf(buffer, "%.10f", 42.895223);
        REQUIRE_STREQ(buffer, "42.8952230000");

        // this testcase checks, that the precision is truncated to 9 digits.
        // a perfect working float should return the whole number
        fmt_sprintf(buffer, "%.12f", 42.89522312345678);
        REQUIRE_STREQ(buffer, "42.895223123000");

        // this testcase checks, that the precision is truncated AND rounded to 9 digits.
        // a perfect working float should return the whole number
        fmt_sprintf(buffer, "%.12f", 42.89522387654321);
        REQUIRE_STREQ(buffer, "42.895223877000");

        fmt_sprintf(buffer, "%6.2f", 42.8952);
        REQUIRE_STREQ(buffer, " 42.90");

        fmt_sprintf(buffer, "%+6.2f", 42.8952);
        REQUIRE_STREQ(buffer, "+42.90");

        fmt_sprintf(buffer, "%+5.1f", 42.9252);
        REQUIRE_STREQ(buffer, "+42.9");

        fmt_sprintf(buffer, "%f", 42.5);
        REQUIRE_STREQ(buffer, "42.500000");

        fmt_sprintf(buffer, "%.1f", 42.5);
        REQUIRE_STREQ(buffer, "42.5");

        fmt_sprintf(buffer, "%f", 42167.0);
        REQUIRE_STREQ(buffer, "42167.000000");

        fmt_sprintf(buffer, "%.9f", -12345.987654321);
        REQUIRE_STREQ(buffer, "-12345.987654321");

        fmt_sprintf(buffer, "%.1f", 3.999);
        REQUIRE_STREQ(buffer, "4.0");

        fmt_sprintf(buffer, "%.0f", 3.5);
        REQUIRE_STREQ(buffer, "4");

        fmt_sprintf(buffer, "%.0f", 4.5);
        REQUIRE_STREQ(buffer, "4");

        fmt_sprintf(buffer, "%.0f", 3.49);
        REQUIRE_STREQ(buffer, "3");

        fmt_sprintf(buffer, "%.1f", 3.49);
        REQUIRE_STREQ(buffer, "3.5");

        fmt_sprintf(buffer, "a%-5.1f", 0.5);
        REQUIRE_STREQ(buffer, "a0.5  ");

        fmt_sprintf(buffer, "a%-5.1fend", 0.5);
        REQUIRE_STREQ(buffer, "a0.5  end");

#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        fmt_sprintf(buffer, "%G", 12345.678);
        REQUIRE_STREQ(buffer, "12345.7");

        fmt_sprintf(buffer, "%.7G", 12345.678);
        REQUIRE_STREQ(buffer, "12345.68");

        fmt_sprintf(buffer, "%.5G", 123456789.);
        REQUIRE_STREQ(buffer, "1.2346E+08");

        fmt_sprintf(buffer, "%.6G", 12345.);
        REQUIRE_STREQ(buffer, "12345.0");

        fmt_sprintf(buffer, "%+12.4g", 123456789.);
        REQUIRE_STREQ(buffer, "  +1.235e+08");

        fmt_sprintf(buffer, "%.2G", 0.001234);
        REQUIRE_STREQ(buffer, "0.0012");

        fmt_sprintf(buffer, "%+10.4G", 0.001234);
        REQUIRE_STREQ(buffer, " +0.001234");

        fmt_sprintf(buffer, "%+012.4g", 0.00001234);
        REQUIRE_STREQ(buffer, "+001.234e-05");

        fmt_sprintf(buffer, "%.3g", -1.2345e-308);
        REQUIRE_STREQ(buffer, "-1.23e-308");

        fmt_sprintf(buffer, "%+.3E", 1.23e+308);
        REQUIRE_STREQ(buffer, "+1.230E+308");
#endif

        // out of range for float
        fmt_sprintf(buffer, "%.1f", 1E20);
        REQUIRE_STREQ(buffer, "%!(exceeded PICO_PRINTF_MAX_FLOAT)");

        // brute force our float against libc float
        for (float i = -100000; i < 100000; i += 1) {
            fmt_sprintf(buffer, "%.5f", i / 10000);
            sprintf(libc_buffer, "%.5f", i / 10000);
            REQUIRE_STREQ(buffer, libc_buffer);
        }

#if PICO_PRINTF_SUPPORT_EXPONENTIAL
        // brute force exp
        for (float i = -1e20f; i < 1e20f; i += 1e15f) {
            fmt_sprintf(buffer, "%.5e", i);
            sprintf(libc_buffer, "%.5e", i);
            REQUIRE_STREQ(buffer, libc_buffer);
        }
#endif
    }
#endif

    TEST_CASE("types", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%i", 0);
        REQUIRE_STREQ(buffer, "0");

        fmt_sprintf(buffer, "%i", 1234);
        REQUIRE_STREQ(buffer, "1234");

        fmt_sprintf(buffer, "%i", 32767);
        REQUIRE_STREQ(buffer, "32767");

        fmt_sprintf(buffer, "%i", -32767);
        REQUIRE_STREQ(buffer, "-32767");

        fmt_sprintf(buffer, "%li", 30L);
        REQUIRE_STREQ(buffer, "30");

        fmt_sprintf(buffer, "%li", -2147483647L);
        REQUIRE_STREQ(buffer, "-2147483647");

        fmt_sprintf(buffer, "%li", 2147483647L);
        REQUIRE_STREQ(buffer, "2147483647");

#if PICO_PRINTF_SUPPORT_LONG_LONG
        fmt_sprintf(buffer, "%lli", 30LL);
        REQUIRE_STREQ(buffer, "30");

        fmt_sprintf(buffer, "%lli", -9223372036854775807LL);
        REQUIRE_STREQ(buffer, "-9223372036854775807");

        fmt_sprintf(buffer, "%lli", 9223372036854775807LL);
        REQUIRE_STREQ(buffer, "9223372036854775807");
#endif

        fmt_sprintf(buffer, "%lu", 100000L);
        REQUIRE_STREQ(buffer, "100000");

        fmt_sprintf(buffer, "%lu", 0xFFFFFFFFL);
        REQUIRE_STREQ(buffer, "4294967295");

#if PICO_PRINTF_SUPPORT_LONG_LONG
        fmt_sprintf(buffer, "%llu", 281474976710656LLU);
        REQUIRE_STREQ(buffer, "281474976710656");

        fmt_sprintf(buffer, "%llu", 18446744073709551615LLU);
        REQUIRE_STREQ(buffer, "18446744073709551615");
#endif

        fmt_sprintf(buffer, "%zu", 2147483647UL);
        REQUIRE_STREQ(buffer, "2147483647");

        fmt_sprintf(buffer, "%zd", 2147483647UL);
        REQUIRE_STREQ(buffer, "2147483647");

        if (sizeof(size_t) == sizeof(long)) {
            fmt_sprintf(buffer, "%zi", -2147483647L);
            REQUIRE_STREQ(buffer, "-2147483647");
        } else {
            fmt_sprintf(buffer, "%zi", -2147483647LL);
            REQUIRE_STREQ(buffer, "-2147483647");
        }

        fmt_sprintf(buffer, "%b", 60000);
        REQUIRE_STREQ(buffer, "1110101001100000");

        fmt_sprintf(buffer, "%lb", 12345678L);
        REQUIRE_STREQ(buffer, "101111000110000101001110");

        fmt_sprintf(buffer, "%o", 60000);
        REQUIRE_STREQ(buffer, "165140");

        fmt_sprintf(buffer, "%lo", 12345678L);
        REQUIRE_STREQ(buffer, "57060516");

        fmt_sprintf(buffer, "%lx", 0x12345678L);
        REQUIRE_STREQ(buffer, "12345678");

#if PICO_PRINTF_SUPPORT_LONG_LONG
        fmt_sprintf(buffer, "%llx", 0x1234567891234567LLU);
        REQUIRE_STREQ(buffer, "1234567891234567");
#endif

        fmt_sprintf(buffer, "%lx", 0xabcdefabL);
        REQUIRE_STREQ(buffer, "abcdefab");

        fmt_sprintf(buffer, "%lX", 0xabcdefabL);
        REQUIRE_STREQ(buffer, "ABCDEFAB");

        fmt_sprintf(buffer, "%c", 'v');
        REQUIRE_STREQ(buffer, "v");

        fmt_sprintf(buffer, "%cv", 'w');
        REQUIRE_STREQ(buffer, "wv");

        fmt_sprintf(buffer, "%s", "A Test");
        REQUIRE_STREQ(buffer, "A Test");

        fmt_sprintf(buffer, "%hhu", 0xFFFFUL);
        REQUIRE_STREQ(buffer, "255");

        fmt_sprintf(buffer, "%hu", 0x123456UL);
        REQUIRE_STREQ(buffer, "13398");

        fmt_sprintf(buffer, "%s%hhi %hu", "Test", 10000, 0xFFFFFFFF);
        REQUIRE_STREQ(buffer, "Test16 65535");

        fmt_sprintf(buffer, "%tx", &buffer[10] - &buffer[0]);
#if PICO_PRINTF_SUPPORT_PTRDIFF_T
        REQUIRE_STREQ(buffer, "a");
#else
        REQUIRE_STREQ(buffer, "%!(unknown specifier='t')x");
#endif

        // TBD
        if (sizeof(intmax_t) == sizeof(long)) {
            fmt_sprintf(buffer, "%ji", -2147483647L);
            REQUIRE_STREQ(buffer, "-2147483647");
        } else {
            fmt_sprintf(buffer, "%ji", -2147483647LL);
            REQUIRE_STREQ(buffer, "-2147483647");
        }
    }

    TEST_CASE("pointer", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%p", (void *) 0x1234U);
        if (sizeof(void *) == 4U) {
            REQUIRE_STREQ(buffer, "00001234");
        } else {
            REQUIRE_STREQ(buffer, "0000000000001234");
        }

        fmt_sprintf(buffer, "%p", (void *) 0x12345678U);
        if (sizeof(void *) == 4U) {
            REQUIRE_STREQ(buffer, "12345678");
        } else {
            REQUIRE_STREQ(buffer, "0000000012345678");
        }

        fmt_sprintf(buffer, "%p-%p", (void *) 0x12345678U, (void *) 0x7EDCBA98U);
        if (sizeof(void *) == 4U) {
            REQUIRE_STREQ(buffer, "12345678-7EDCBA98");
        } else {
            REQUIRE_STREQ(buffer, "0000000012345678-000000007EDCBA98");
        }

        if (sizeof(uintptr_t) == sizeof(uint64_t)) {
            fmt_sprintf(buffer, "%p", (void *) (uintptr_t) 0xFFFFFFFFU);
            REQUIRE_STREQ(buffer, "00000000FFFFFFFF");
        } else {
            fmt_sprintf(buffer, "%p", (void *) (uintptr_t) 0xFFFFFFFFU);
            REQUIRE_STREQ(buffer, "FFFFFFFF");
        }
    }

    TEST_CASE("unknown flag", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%kmarco", 42, 37);
        REQUIRE_STREQ(buffer, "%!(unknown specifier='k')marco");
    }

    TEST_CASE("string length", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%.4s", "This is a test");
        REQUIRE_STREQ(buffer, "This");

        fmt_sprintf(buffer, "%.4s", "test");
        REQUIRE_STREQ(buffer, "test");

        fmt_sprintf(buffer, "%.7s", "123");
        REQUIRE_STREQ(buffer, "123");

        fmt_sprintf(buffer, "%.7s", "");
        REQUIRE_STREQ(buffer, "");

        fmt_sprintf(buffer, "%.4s%.2s", "123456", "abcdef");
        REQUIRE_STREQ(buffer, "1234ab");

        fmt_sprintf(buffer, "%.4.2s", "123456");
        REQUIRE_STREQ(buffer, "%!(unknown specifier='.')2s");

        fmt_sprintf(buffer, "%.*s", 3, "123456");
        REQUIRE_STREQ(buffer, "123");
    }

    TEST_CASE("buffer length", "[]");
    {
        char buffer[100];
        int ret;

        ret = fmt_snprintf(NULL, 10, "%s", "Test");
        REQUIRE(ret == 4);
        ret = fmt_snprintf(NULL, 0, "%s", "Test");
        REQUIRE(ret == 4);

        buffer[0] = (char) 0xA5;
        ret = fmt_snprintf(buffer, 0, "%s", "Test");
        REQUIRE(buffer[0] == (char) 0xA5);
        REQUIRE(ret == 4);

        buffer[0] = (char) 0xCC;
        fmt_snprintf(buffer, 1, "%s", "Test");
        REQUIRE(buffer[0] == '\0');

        fmt_snprintf(buffer, 2, "%s", "Hello");
        REQUIRE_STREQ(buffer, "H");
    }

    TEST_CASE("ret value", "[]");
    {
        char buffer[100];
        int ret;

        ret = fmt_snprintf(buffer, 6, "0%s", "1234");
        REQUIRE_STREQ(buffer, "01234");
        REQUIRE(ret == 5);

        ret = fmt_snprintf(buffer, 6, "0%s", "12345");
        REQUIRE_STREQ(buffer, "01234");
        REQUIRE(ret == 6); // '5' is truncated

        ret = fmt_snprintf(buffer, 6, "0%s", "1234567");
        REQUIRE_STREQ(buffer, "01234");
        REQUIRE(ret == 8); // '567' are truncated

        ret = fmt_snprintf(buffer, 10, "hello, world");
        REQUIRE(ret == 12);

        ret = fmt_snprintf(buffer, 3, "%d", 10000);
        REQUIRE(ret == 5);
        REQUIRE(strlen(buffer) == 2U);
        REQUIRE(buffer[0] == '1');
        REQUIRE(buffer[1] == '0');
        REQUIRE(buffer[2] == '\0');
    }

    TEST_CASE("misc", "[]");
    {
        char buffer[100];

        fmt_sprintf(buffer, "%u%u%ctest%d %s", 5, 3000, 'a', -20, "bit");
        REQUIRE_STREQ(buffer, "53000atest-20 bit");

#if PICO_PRINeTF_SUPPORT_FLOAT
        fmt_sprintf(buffer, "%.*f", 2, 0.33333333);
        REQUIRE_STREQ(buffer, "0.33");
#endif

        fmt_sprintf(buffer, "%.*d", -1, 1);
        REQUIRE_STREQ(buffer, "1");

        fmt_sprintf(buffer, "%.3s", "foobar");
        REQUIRE_STREQ(buffer, "foo");

        fmt_sprintf(buffer, "% .0d", 0);
        REQUIRE_STREQ(buffer, " ");

        fmt_sprintf(buffer, "%10.5d", 4);
        REQUIRE_STREQ(buffer, "     00004");

        fmt_sprintf(buffer, "%*sx", -3, "hi");
        REQUIRE_STREQ(buffer, "hi x");

#if PICO_PRINTF_SUPPORT_FLOAT && PICO_PRINTF_SUPPORT_EXPONENTIAL
        fmt_sprintf(buffer, "%.*g", 2, 0.33333333);
        REQUIRE_STREQ(buffer, "0.33");

        fmt_sprintf(buffer, "%.*e", 2, 0.33333333);
        REQUIRE_STREQ(buffer, "3.33e-01");
#endif
    }

    if (failures) {
        printf("%u failures\n", failures);
        return 1;
    } else {
        printf("success!\n");
        return 0;
    }
}
