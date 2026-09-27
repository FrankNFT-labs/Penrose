# Copyright 2019 Tyler Coy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

TARGET := test
SOURCES := \
	test/*.cpp \
	app/pitch.cpp \

TGT_DEFS := \
	__int24=int32_t \
	__uint24=uint32_t \

CPPFLAGS := -g -Wall -Wextra -iquote .
TGT_CFLAGS := $(CPPFLAGS) -std=gnu11
# GoogleTest 1.17 and later require C++17, and the tests already use it.
TGT_CXXFLAGS := $(CPPFLAGS) -std=gnu++17 -pthread -Wold-style-cast

TGT_LDLIBS := -lgtest -lpthread -lz -lgtest_main

ifeq ($(shell uname -s),Darwin)
# Homebrew's GoogleTest lives outside the default search paths. It is built
# with libc++, so the tests must be compiled with clang, the platform default.
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
TGT_CXXFLAGS += -isystem $(BREW_PREFIX)/include
TGT_LDFLAGS := -L$(BREW_PREFIX)/lib
endif

.PHONY: tests
tests: $(TARGET_DIR)/$(TARGET)

.PHONY: check
check: $(TARGET_DIR)/$(TARGET)
	$<
