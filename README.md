<!--
  Copyright (c) 2014-2015, 2017-2021  Marco Paland (info@paland.com)
  SPDX-License-Identifier: MIT

  Copyright (c) 2025  Luke T. Shumaker
  SPDX-License-Identifier: BSD-3-Clause
  -->

# pico-fmt : A `printf` implementation for embedded systems

[![Quality Assurance](https://github.com/LukeShu/pico-fmt/actions/workflows/qa.yml/badge.svg)](https://github.com/LukeShu/pico-fmt/actions/workflows/qa.yml)

This is a fork of [Raspberry Pi Pico SDK's
fork](https://github.com/raspberrypi/pico-sdk/tree/master/src/rp2_common/pico_printf)
of [Marco Paland's `printf`](https://github.com/mpaland/printf).

> I have split the history of Raspberry Pi's modifications out onto
> Marco's original repository, so that one can see in the Git history
> what changes Raspberry Pi has made.  The `pico-sdk` branch tracks
> what is in pico-sdk.

 - The `pico_fmt` directory provides several `fmt_*` functions such as
  `fmt_vfctprintf()`.
 - The `pico_printf` directory wraps `pico_fmt`, providing a drop-in
   replacement for the pico-sdk's `pico_printf` package (adding
   non-`fmt_`-prefixed aliases an hooking in to `pico_stdio`).

You do not need to use pico-sdk to use `pico_fmt`.

# Usage

## Without pico-sdk

 - Add `pico_fmt/include/` to your C-preprocessor include path.
 - Add/link `pico_fmt/*.c` in to your project.
 - Include `<pico/fmt_printf.h>`, which will give you:

   ```c
   typedef void (*fmt_fct_t)(char character, void *arg);

   // vprintf with output function
   int fmt_vfctprintf(fmt_fct_t out, void *arg, const char *format, va_list va);

   // printf with output function
   int fmt_fctprintf(fmt_fct_t out, void *arg, const char *format, ...);

   // 1:1 with the <stdio.h> non-`fmt_` versions:
   int fmt_vsnprintf(char *buffer, size_t count, const char *format, va_list);
   int fmt_snprintf(char *buffer, size_t count, const char *format, ...);
   int fmt_vsprintf(char *buffer, const char *format, va_list);
   int fmt_sprintf(char *buffer, const char *format, ...);
   ```

`pico_fmt` has no concept of "stdout"; you may define "stdout"
wrappers with something like:

```c
void _putchar(char character, void *) {
    // send character to console et c.
}

int vprintf(const char *format, va_list va) {
    return fmt_vfctprintf(_putchar, NULL, format, va);
}

int printf(const char *format, ...) {
    va_list va;
    va_start(va, format);
    const int ret = vprintf(format, va);
    va_end(va);
    return ret;
}
```

## With pico-sdk (CMake)

 - Before calling `pico_sdk_init()`, call `add_subdirectory(...)` on
   both `pico_fmt` and `pico_printf`:
   ```cmake
   set(PICO_SDK_PATH "${CMAKE_SOURCE_DIR}/3rd-party/pico-sdk")
   include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
   # ...
   add(subdirectory("${CMAKE_SOURCE_DIR}/3rd-party/pico-fmt/pico_fmt")
   add(subdirectory("${CMAKE_SOURCE_DIR}/3rd-party/pico-fmt/pico_printf")
   # ...
   pico_sdk_init()
   ```

 - You may use `pico_fmt` either by including `<pico/fmt_printf.h>`
   and using the `fmt_*` functions, or by using the usual
   `pico_printf`/`pico_stdio`:

    * `<pico/fmt_printf.h>` gives you:
      ```c
      typedef void (*fmt_fct_t)(char character, void *arg);

      // vprintf with output function
      int fmt_vfctprintf(fmt_fct_t out, void *arg, const char *format, va_list va);

      // printf with output function
      int fmt_fctprintf(fmt_fct_t out, void *arg, const char *format, ...);

      // 1:1 with the <stdio.h> non-`fmt_` versions:
      int fmt_vsnprintf(char *buffer, size_t count, const char *format, va_list);
      int fmt_snprintf(char *buffer, size_t count, const char *format, ...);
      int fmt_vsprintf(char *buffer, const char *format, va_list);
      int fmt_sprintf(char *buffer, const char *format, ...);
      ```

    * `pico_printf` still (like vanilla pico-sdk `pico_printf`) uses
      the `pico_set_printf_implementation(${TARGET} ${IMPL})` toggle
      to set which `printf` implementation to use;
       + `pico` and `default`: Use `pico_fmt`
          - If `PICO_PRINTF_ALWAYS_INCLUDED=0` (the default), then it
            may be pruned out of the binary if you don't actually call
            printf (even though internal parts of pico-sdk may call
            printf).  There is some run-time overhead to this.
          - If `PICO_PRINTF_ALWAYS_INCLUDED=1`, then it will always be
            included, even if it is only used by internal parts of
            pico-sdk.  This avoids run-time overhead.
       + `compiler`: Use the compiler/libc default.
       + `none`: Panic if any `printf` routines are called.

## With pico-sdk (Bazel)

I dislike Bazel even more than I dislike CMake; I do not provide Bazel
build files for pico-fmt (but contributions welcome!).

# Size

<!-- BEGIN ./build-aux/measure output -->
With arm-none-eabi-gcc version `arm-none-eabi-gcc (Arch Repository) 14.2.0`:

  Debug = `-mcpu=cortex-m0plus -mthumb -g -Og`:
  |                         |                  text                   |               rodata                |              max_stack              |
  |-------------------------|-----------------------------------------|-------------------------------------|-------------------------------------|
  |                         | mini + ptr +  ll + float +  exp =   tot | mini + ptr + ll + float + exp = tot | mini + ptr + ll + float + exp = tot |
  |-------------------------|-----------------------------------------|-------------------------------------|-------------------------------------|
  | pico-sdk 2.1.1 Debug    | 2288 +   8 + 916 +  4552 + 3868 = 11632 |  336 +  76 +  0 +   108 +   0 = 520 |  320 +   0 + 48 +     0 + 208 = 576 |
  | pico-fmt Git-main Debug | 2288 +   8 + 916 +  4552 + 3868 = 11632 |  336 +  76 +  0 +   108 +   0 = 520 |  320 +   0 + 48 +     0 + 208 = 576 |

  Release = `-mcpu=cortex-m0plus -mthumb -g -O3 -DNDEBUG`:
  |                           |                  text                   |               rodata                |              max_stack              |
  |---------------------------|-----------------------------------------|-------------------------------------|-------------------------------------|
  |                           | mini + ptr +  ll + float +  exp =   tot | mini + ptr + ll + float + exp = tot | mini + ptr + ll + float + exp = tot |
  |---------------------------|-----------------------------------------|-------------------------------------|-------------------------------------|
  | pico-sdk 2.1.1 Release    | 3440 +  64 + 756 +  4852 + 4236 = 13348 |  336 +  76 +  0 +   108 +   0 = 520 |  228 +   8 + 48 +    36 + 336 = 656 |
  | pico-fmt Git-main Release | 3440 +  64 + 756 +  4852 + 4236 = 13348 |  336 +  76 +  0 +   108 +   0 = 520 |  228 +   8 + 48 +    36 + 336 = 656 |

  MinSizeRel = `-mcpu=cortex-m0plus -mthumb -g -Os -DNDEBUG`:
  |                              |                  text                   |               rodata                |              max_stack               |
  |------------------------------|-----------------------------------------|-------------------------------------|--------------------------------------|
  |                              | mini + ptr +  ll + float +  exp =   tot | mini + ptr + ll + float + exp = tot | mini + ptr +  ll + float + exp = tot |
  |------------------------------|-----------------------------------------|-------------------------------------|--------------------------------------|
  | pico-sdk 2.1.1 MinSizeRel    | 1832 +   4 + 740 +  4532 + 3796 = 10904 |    0 +   0 +  0 +   104 +   0 = 104 |  220 +   0 + 128 +    76 + 160 = 584 |
  | pico-fmt Git-main MinSizeRel | 1832 +   4 + 740 +  4532 + 3796 = 10904 |    0 +   0 +  0 +   104 +   0 = 104 |  220 +   0 + 128 +    76 + 160 = 584 |
<!-- END ./build-aux/measure output -->

These measurements are for the printf code compiled stand-alone
against libgcc; when used with `pico_float` or other `__aeabi_*`
function providers, the numbers may be different.  The max_stack
measurement obviously does not take in to account your output
function.  The 'data' and 'bss' sections are not shown in the table
because they always have size zero; the code does not make use of
globals.

# License

pico-fmt as a whole is subject to both the MIT license
(`SPDX-License-Identifier: MIT`) and the 3-clause BSD license
(`SPDX-License-Identifier: BSD-3-Clause`); see `LICENSE.txt` for the
full text of each license.

Each individual file has `SPDX-License-Identifier:` tags indicating
precisely which license(s) that specific file is subject to.
