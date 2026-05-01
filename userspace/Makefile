CROSS_COMPILE ?= i686-elf-
CC := $(CROSS_COMPILE)gcc

ROOT := $(CURDIR)
SRC_DIR := $(ROOT)/src
BIN_DIR := $(ROOT)/bin
UTIL_DIR := $(ROOT)/util
LIB_ENTRY := $(UTIL_DIR)/lib/main.c
LDSCRIPT := $(UTIL_DIR)/user.ld
INCLUDE_DIR := $(ROOT)/../libs/inc

SOURCES := $(shell find "$(SRC_DIR)" -name "*.c" -type f | sort)
HEADERS := $(shell find "$(INCLUDE_DIR)" -name "*.h" -type f | sort)
ELFS := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.elf,$(SOURCES))

CFLAGS := -I$(INCLUDE_DIR) -m32 -nostdlib -static -ffreestanding -Wall -Wextra -Werror
LDFLAGS := -T $(LDSCRIPT)

.PHONY: all clean

all: $(ELFS)

$(BIN_DIR)/%.elf: $(SRC_DIR)/%.c $(LIB_ENTRY) $(LDSCRIPT) $(HEADERS)
	@mkdir -p "$(@D)"
	@echo "[USER] $<"
	$(CC) $(CFLAGS) $(LDFLAGS) -o "$@" "$<" "$(LIB_ENTRY)"

clean:
	rm -f $(ELFS)
