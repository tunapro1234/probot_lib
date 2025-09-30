# Unified Makefile for building examples with arduino-cli

# Board/toolchain
FQBN        ?= esp32:esp32:esp32s3
PORT        ?= /dev/ttyACM0
BAUD        ?= 115200
ARDUINO_CLI ?= arduino-cli

# Examples
EXAMPLES_DIR   := $(CURDIR)/examples
EXAMPLES_LIST  := $(filter-out __library_impl,$(notdir $(wildcard $(EXAMPLES_DIR)/*)))
DEFAULT_EXAMPLE := $(firstword $(EXAMPLES_LIST))
EXAMPLE        ?= $(DEFAULT_EXAMPLE)
BUILD_DIR_BASE := $(CURDIR)/.build

TEST_STUB_DIR := $(CURDIR)/tests/stubs
TEST_SOURCES := $(filter %.cpp,$(wildcard $(CURDIR)/tests/*.cpp))

# Common flags
EXTRA_FLAGS_COMMON := -DESP32S3 -DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1

# Example-specific extra flags (function)
example_flags = $(if $(filter LoopPeriodStress,$(1)),-DPROBOT_CLM_NOLOG=1 -DPROBOT_SCHED_NOLOG=1,)

.PHONY: all build build-all _build_single upload clean boards libs serial list help test tests/control_tests

all: help

help:
	@echo "Usage: make build EXAMPLE=<Name|all>"
	@echo "Examples directory: $(EXAMPLES_DIR)"
	@echo "Targets: build, build-all, upload, serial, clean, list"
	@echo "Build artifacts: $(BUILD_DIR_BASE)/<Example>"

list:
ifeq ($(EXAMPLES_LIST),)
	@echo "(no examples found)"
else
	@for ex in $(EXAMPLES_LIST); do echo $$ex; done
endif

build:
ifeq ($(EXAMPLE),all)
	@set -e; \
	for ex in $(EXAMPLES_LIST); do \
	  $(MAKE) --no-print-directory _build_single EXAMPLE=$$ex; \
	done
else
	@$(MAKE) --no-print-directory _build_single EXAMPLE=$(EXAMPLE)
endif

build-all:
	@$(MAKE) build EXAMPLE=all

_build_single:
	@echo "==> Building $(EXAMPLE)"
	$(ARDUINO_CLI) compile --fqbn $(FQBN) --warnings all \
	  --library $(CURDIR) \
	  --build-path $(BUILD_DIR_BASE)/$(EXAMPLE) \
	  --build-property compiler.cpp.extra_flags="$(EXTRA_FLAGS_COMMON) $(call example_flags,$(EXAMPLE))" \
	  --build-property compiler.c.extra_flags="$(EXTRA_FLAGS_COMMON) $(call example_flags,$(EXAMPLE))" \
	  $(EXAMPLES_DIR)/$(EXAMPLE)

upload: build
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(FQBN) $(EXAMPLES_DIR)/$(EXAMPLE)

serial:
	arduino-cli monitor -p $(PORT) --config $(BAUD)

clean:
	rm -rf $(BUILD_DIR_BASE)
	-$(ARDUINO_CLI) cache clean >/dev/null 2>&1 || true
	rm -f tests/control_tests

boards:
	arduino-cli board list

libs:
	arduino-cli lib install "Adafruit NeoPixel" 

tests/control_tests: $(TEST_SOURCES)
	g++ -std=c++17 -Wall -Wextra -pedantic -I src -I $(TEST_STUB_DIR) -DPROBOT_CLM_NOLOG=1 -DPROBOT_SCHED_NOLOG=1 -o $@ $(TEST_SOURCES)

test: build tests/control_tests
	./tests/control_tests
