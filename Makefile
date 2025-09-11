# Unified Makefile for building examples with arduino-cli

# Board/toolchain
FQBN        := esp32:esp32:esp32s3
PORT        := /dev/ttyACM0
BAUD        := 115200

# Examples
EXAMPLES_DIR:= $(CURDIR)/examples
EXAMPLE     := Basic
SKETCH_DIR  := $(EXAMPLES_DIR)/$(EXAMPLE)

# Common flags
EXTRA_FLAGS_COMMON := -DESP32S3 -DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1
EXTRA_FLAGS        :=

# Auto flags per example
ifeq ($(EXAMPLE),LoopPeriodStress)
EXTRA_FLAGS += -DPROBOT_CLM_NOLOG=1 -DPROBOT_SCHED_NOLOG=1
endif

.PHONY: all build upload clean boards libs serial list help

all: help

help:
	@echo "Usage: make build EXAMPLE=<Name>"
	@echo "Examples directory: $(EXAMPLES_DIR)"
	@echo "Targets: build, upload, serial, clean, list"

list:
	@ls -1 $(EXAMPLES_DIR)

build:
	arduino-cli compile --fqbn $(FQBN) --warnings all \
	  --library $(CURDIR) \
	  --build-property compiler.cpp.extra_flags="$(EXTRA_FLAGS_COMMON) $(EXTRA_FLAGS)" \
	  --build-property compiler.c.extra_flags="$(EXTRA_FLAGS_COMMON) $(EXTRA_FLAGS)" \
	  $(SKETCH_DIR)

upload: build
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH_DIR)

serial:
	arduino-cli monitor -p $(PORT) --config $(BAUD)

clean:
	rm -rf $(HOME)/.cache/arduino/sketches/*

boards:
	arduino-cli board list

libs:
	arduino-cli lib install "Adafruit NeoPixel" 