# Directory
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# Compilation Configuration
CC := gcc
IFLAGS := -lpthread -lm

# Detect Architecture and Set Flags Accordingly
ARCH := $(shell uname -m)

ifeq ($(ARCH), x86_64)
    CFLAGS := -mavx -mavx2 -g -O3
    $(info Compiling for x86_64 with AVX2 support)
else ifeq ($(ARCH), aarch64)
    CFLAGS := -march=armv8-a -g -O3
    $(info Compiling for ARM64 with NEON support)
else ifeq ($(ARCH), armv7l)
    CFLAGS := -mfpu=neon -g -O3
    $(info Compiling for ARMv7 with NEON support)
else
    CFLAGS := -g -O3
    $(info Compiling for generic architecture)
endif

# File and directory names
BUILD_DIR := $(ROOT_DIR)/build
SRC_DIR := $(ROOT_DIR)/src

# Get all possible benchmarks
BENCHMARKS := $(notdir $(shell dirname $(shell find $(SRC_DIR)/ -mindepth 2 -maxdepth 2 -name "Makefile.mk")))

# Output
BINS_BM  := $(addprefix $(BUILD_DIR)/,$(BENCHMARKS))
CLEAN_BM := $(addprefix clean_,$(BENCHMARKS))

# Default
all: $(BINS_BM)

# Build directory
$(BUILD_DIR):
	mkdir -p $@

# Clean
clean: $(CLEAN_BM)
	rm -rf $(BUILD_DIR)

# Template
include template.mk

# All benchmarks/applications
-include $(SRC_DIR)/Makefile.mk

.PHONY: clean all
