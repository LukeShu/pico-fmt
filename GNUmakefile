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

sources_py3 = build-aux/measure

lint:
	mypy --strict --scripts-are-modules $(sources_py3)
	black --line-length 100 --check $(sources_py3)
	isort --check $(sources_py3)
.PHONY: lint

format:
	black --line-length 100 $(sources_py3)
	isort $(sources_py3)
.PHONY: format

################################################################################

update-readme:
	{ \
	  sed '/<!-- BEGIN \.\/build-aux\/measure output -->/q;' README.md && \
	  ./build-aux/measure && \
	  sed -n '/<!-- END \.\/build-aux\/measure output -->/,$$p;' README.md && \
	:; } > README.md.new
	mv README.md.new README.md
.PHONY: update-readme
