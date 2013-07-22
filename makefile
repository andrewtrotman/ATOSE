#
# Makefile for ATOSE and the associated tools that run on the i.MX6Q
# Copyright (c) 2013 Andrew Trotman
#
# Assumes GNU make
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
IMX6Q_TOOLS =						         \
	$(BIN_DIR)/imx6q_usb.elf	         \
	$(BIN_DIR)/imx6q_uart.elf	         \
	$(BIN_DIR)/imx6q_interrupt.elf		\
	$(BIN_DIR)/imx6q_clock.elf		\
	$(BIN_DIR)/imx6q_ram.elf		\
	$(BIN_DIR)/imx6q_timer.elf

all : $(BIN_DIR) $(OBJ_DIR) $(IMX6Q_TOOLS)

$(BIN_DIR) :
	mkdir $(BIN_DIR)

$(OBJ_DIR) :
	mkdir $(OBJ_DIR)

$(BIN_DIR)/%.elf : $(TESTS_DIR)/%.c
	@echo $@
	$(CC) $(CCFLAGS) -o $@ $(TESTS_DIR)/imx6q.s $< $(CLINKFLAGS) -T $(TESTS_DIR)/imx6q.ld

clean:
	rm $(IMX6Q_TOOLS)