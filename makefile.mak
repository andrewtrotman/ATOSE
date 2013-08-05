#
# Generic makefile parts for ATOSE
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Choose a Target for ATOSE
#
TARGET = PINNULE
CPU = IMX6Q

#
#	Now the compilers, etc.
#
AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a9

CC = arm-none-eabi-gcc
CCFLAGS = -mcpu=cortex-a9 -D$(CPU) -D$(TARGET) -g3 -Wall -Os -l gcc -ffreestanding -nostartfiles

#
#	Directories
#
OBJ_DIR = obj
BIN_DIR = bin
TOOLS_DIR = tools
TESTS_DIR = tests
EXAMPLES_DIR = examples

#
#	Tools for testing subsystems of the i.MX6Q
#
IMX6Q_TOOLS =						     \
	$(BIN_DIR)/imx6q_usb.elf	         \
	$(BIN_DIR)/imx6q_uart.elf	         \
	$(BIN_DIR)/imx6q_interrupt.elf		 \
	$(BIN_DIR)/imx6q_clock.elf		     \
	$(BIN_DIR)/imx6q_ram.elf		     \
	$(BIN_DIR)/imx6q_timer.elf

#
#	Tool that help with the host PC, Mac, etc.  This includes debugging tools and uploaders
#
HOST_TOOLS = $(HOST_TOOLS)            \
	$(BIN_DIR)/bin_to_c.$(EXT)        \
	$(BIN_DIR)/elf_reader.$(EXT)      \
	$(BIN_DIR)/imx_run.$(EXT)         \
	$(BIN_DIR)/imximage_render.$(EXT)

CLEANABLE = $(HOST_TOOLS) $(IMX6Q_TOOLS)

all : $(BIN_DIR) $(OBJ_DIR) $(IMX6Q_TOOLS) $(HOST_TOOLS)

$(BIN_DIR) :
	mkdir $(BIN_DIR)

$(OBJ_DIR) :
	mkdir $(OBJ_DIR)
