#
# Generic (not OS specific) makefile parts for ATOSE
#
# Copyright (c) 2013 Andrew Trotman
#

#
#	Tell make about asm files
#
.SUFFIXES: .asm

#
#	Choose a Target for ATOSE
#	Possible targets are: (SABRE_LITE, PINNULE)
#
TARGET = SABRE_LITE
CPU = IMX6Q

#
#	Now the compilers, etc.
#
AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a9

CC = arm-none-eabi-gcc
CCFLAGS = -mcpu=cortex-a9 -D$(CPU) -D$(TARGET) -g3 -Wall -Os -l gcc -ffreestanding -nostartfiles

CCXX = arm-none-eabi-g++
CCXXFLAGS = -std=c++0x $(CCFLAGS) -fno-exceptions -fno-rtti

#
#	Directories
#
OBJ_DIR = obj
BIN_DIR = bin
TOOLS_DIR = tools
TESTS_DIR = tests
EXAMPLES_DIR = examples
SOURCE_DIR = source-imx6

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
HOST_TOOLS = $(HOST_TOOLS_EXTRAS)            \
	$(BIN_DIR)/bin_to_c.$(EXT)        \
	$(BIN_DIR)/elf_reader.$(EXT)      \
	$(BIN_DIR)/imx_run.$(EXT)         \
	$(BIN_DIR)/imximage_render.$(EXT)

#
# WARNING -> atose_startup.o *must* be the first object in the list so that imx_run.exe can load and run the program
#
ATOSE_OBJECTS =								\
	$(OBJ_DIR)/atose_startup.o				\
	$(OBJ_DIR)/atose_api.o					\
	$(OBJ_DIR)/address_space.o				\
	$(OBJ_DIR)/atose.o						\
	$(OBJ_DIR)/client_file.o				\
	$(OBJ_DIR)/clock_imx6q.o				\
	$(OBJ_DIR)/cpu_arm.o					\
	$(OBJ_DIR)/cpu_arm_imx6q.o				\
	$(OBJ_DIR)/ctypes.o						\
	$(OBJ_DIR)/debug.o						\
	$(OBJ_DIR)/fat.o						\
	$(OBJ_DIR)/file_system.o				\
	$(OBJ_DIR)/host_usb.o					\
	$(OBJ_DIR)/host_usb_device.o			\
	$(OBJ_DIR)/host_usb_device_disk.o		\
	$(OBJ_DIR)/host_usb_device_hub.o		\
	$(OBJ_DIR)/idle.o						\
	$(OBJ_DIR)/interrupt.o					\
	$(OBJ_DIR)/interrupt_arm_gic.o			\
	$(OBJ_DIR)/kernel_malloc.o				\
	$(OBJ_DIR)/kernel_memory_allocator.o	\
	$(OBJ_DIR)/main.o						\
	$(OBJ_DIR)/malloc.o						\
	$(OBJ_DIR)/mmu.o						\
	$(OBJ_DIR)/mmu_page_list.o				\
	$(OBJ_DIR)/pipe.o						\
	$(OBJ_DIR)/pipe_test.o					\
	$(OBJ_DIR)/process_manager.o			\
	$(OBJ_DIR)/process_allocator.o			\
	$(OBJ_DIR)/semaphore.o					\
	$(OBJ_DIR)/server_disk.o				\
	$(OBJ_DIR)/sleep_wake.o					\
	$(OBJ_DIR)/stack.o						\
	$(OBJ_DIR)/timer_imx6q.o				\
	$(OBJ_DIR)/uart_imx6q.o					\
	$(OBJ_DIR)/usb.o						\
	$(OBJ_DIR)/usb_imx6q.o

CLEANABLE = $(HOST_TOOLS) $(IMX6Q_TOOLS) $(ATOSE_OBJECTS)

all : $(BIN_DIR) $(OBJ_DIR) $(IMX6Q_TOOLS) $(HOST_TOOLS) $(BIN_DIR)/atose.elf

$(BIN_DIR)/atose.elf : $(ATOSE_OBJECTS) $(SOURCE_DIR)/imx6q.ld
	@echo $@
	@$(CC) $(CFLAGS) -o $(BIN_DIR)/atose.elf $(ATOSE_OBJECTS) -T $(SOURCE_DIR)/imx6q.ld

$(BIN_DIR) :
	@mkdir $(BIN_DIR)

$(OBJ_DIR) :
	@mkdir $(OBJ_DIR)


include makefile.depend
