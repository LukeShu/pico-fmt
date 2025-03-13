# Copyright (C) 2025  Luke T. Shumaker <lukeshu@lukeshu.com>
# SPDX-License-Identifier: BSD-3-Clause

all: build
.PHONY: all

.NOTINTERMEDIATE:
.DELETE_ON_ERROR:

################################################################################

CFLAGS += -Og

CFLAGS += -Werror

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wnull-dereference
CFLAGS += -Wuninitialized
CFLAGS += -Wunused
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wfloat-equal
CFLAGS += -Wmissing-format-attribute
CFLAGS += -Wsign-compare
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wredundant-decls
CFLAGS += -Wswitch-enum

export CFLAGS

################################################################################

build/Makefile: $(MAKEFILE_LIST)
	mkdir -p $(@D) && cd $(@D) && cmake -DCMAKE_BUILD_TYPE=Debug ..
build: build/Makefile
	$(MAKE) -C $(<D)
.PHONY: build

check: build
	$(MAKE) -C build test
.PHONY: check

################################################################################

sources_c  = pico_fmt/printf.c
sources_c += pico_fmt/include/pico/fmt_printf.h
sources_c += pico_fmt/test/test_suite.c
#sources_c += pico_printf/printf_pico.c
#sources_c += pico_printf/include/pico/printf.h
sources_py3 = build-aux/measure

lint:
	$(MAKE) -k lint/c lint/py3
lint/c:
	clang-format -Werror --dry-run -- $(sources_c)
lint/py3:
	mypy --strict --scripts-are-modules $(sources_py3)
	black --line-length 100 --check $(sources_py3)
	isort --check $(sources_py3)
.PHONY: lint lint/c lint/py3

format:
	$(MAKE) -k format/c format/py3
format/c:
	clang-format -Werror -i -- $(sources_c)
format/py3:
	black --line-length 100 $(sources_py3)
	isort $(sources_py3)
.PHONY: format format/c format/py3

################################################################################

update-readme:
	{ \
	  sed '/<!-- BEGIN \.\/build-aux\/measure output -->/q;' README.md && \
	  ./build-aux/measure && \
	  sed -n '/<!-- END \.\/build-aux\/measure output -->/,$$p;' README.md && \
	:; } > README.md.new
	mv README.md.new README.md
.PHONY: update-readme
