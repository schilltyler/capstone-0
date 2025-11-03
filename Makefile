# Makefile for building the modular agent (no libc)
# Always builds for aarch64 - auto-detects if cross-compilation is needed

# Configuration
SERVER_IP ?= 100.64.0.13
SERVER_PORT ?= 4444
PASSWORD ?= DEADBEEF

# TODO commands - students can uncomment these as they implement each command
# This will keep the stub implementation that returns "not yet implemented"
# Remove a command from this list once you've implemented it
#TODO_CMDS = tailf sed bgrep timestomp wc djb2sum runrwx proc_maps
TODO_CMDS = tailf sed timestomp wc djb2sum runrwx proc_maps
#TODO_CMDS =
# Generate -DTODO_CMD_<name> flags for each TODO command
TODO_FLAGS = $(foreach cmd,$(TODO_CMDS),-DTODO_CMD_$(cmd))

# Detect if we need cross-compilation
HOST_ARCH := $(shell uname -m)
ifeq ($(HOST_ARCH),aarch64)
    # Native aarch64 build
    CC = gcc
    #STRIP = strip
else
    # Cross-compile for aarch64
    CC = aarch64-linux-gnu-gcc
    #STRIP = aarch64-linux-gnu-strip
endif
CFLAGS = -ggdb -nostdlib -static -fno-stack-protector -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -O1 -Wall -Wextra -Wpedantic -Wvla -Wshadow -Iinc $(TODO_FLAGS)
LDFLAGS = -static -nostdlib -fsanitize=address,undefined

# Source files
SRC_DIR = src
CMD_DIR = $(SRC_DIR)/cmds
INC_DIR = inc
OBJ_DIR = obj
BIN_DIR = bin
BIN_TARGET = $(BIN_DIR)/agent

# Command source files
CMD_SRCS = $(wildcard $(CMD_DIR)/*.c)

# Main sources
MAIN_SRCS = $(SRC_DIR)/agent.c $(SRC_DIR)/utils.c

# All sources
ALL_SRCS = $(MAIN_SRCS) $(CMD_SRCS)

# Object files
CMD_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CMD_SRCS))
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(MAIN_SRCS))
ALL_OBJS = $(MAIN_OBJS) $(CMD_OBJS)

# Headers
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Targets
.PHONY: all clean config agent

all: config agent

config: inc/config.h

inc/config.h:
	@echo "[*] Generating inc/config.h..."
	python3 builder.py $(SERVER_IP) $(SERVER_PORT) $(PASSWORD)

# Compile each source file to object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c inc/config.h $(HEADERS)
	@echo "[*] Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link all object files into final agent
agent: $(BIN_TARGET)

$(BIN_TARGET): inc/config.h $(ALL_OBJS)
	@echo "[*] Linking agent..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $(ALL_OBJS)
	@echo "[+] Agent built successfully"
	@echo "    Size: $$(stat -c%s $@) bytes"
	#$(STRIP) $@
	@echo "    Size (stripped): $$(stat -c%s $@) bytes"

clean:
	@echo "[*] Cleaning..."
	rm -rf $(BIN_DIR) $(OBJ_DIR) inc/config.h downloaded_*

# Test target
test: agent
	@echo "[*] Starting server in background..."
	@python3 server.py $(SERVER_PORT) &
	@sleep 1
	@echo "[*] Connecting with agent..."
	@$(BIN_TARGET)

# Help
help:
	@echo "Usage:"
	@echo "  make                    - Build agent with default config"
	@echo "  make SERVER_IP=<ip> SERVER_PORT=<port> PASSWORD=<pass>"
	@echo "                          - Build with custom config"
	@echo "  make clean              - Clean build artifacts"
	@echo ""
	@echo "Incremental Development:"
	@echo "  Edit TODO_CMDS at the top of Makefile to control which"
	@echo "  commands use stub implementations vs your code."
	@echo ""
	@echo "  Commands in TODO_CMDS list will return 'not yet implemented'"
	@echo "  Remove a command from the list once you've implemented it!"
	@echo ""
	@echo "  Current TODO commands: $(TODO_CMDS)"
	@echo ""
	@echo "Example:"
	@echo "  make SERVER_IP=192.168.1.100 SERVER_PORT=4444 PASSWORD=DEADBEEF"
	@echo ""
	@echo "Then run:"
	@echo "  python3 server.py 4444"
	@echo "  $(BIN_TARGET)"
