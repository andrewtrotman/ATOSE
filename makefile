#
# Makefile for ATOSE and the associated tools
#
CC = arm-none-eabi-g++
CFLAGS = -mcpu=arm926ej-s -ffreestanding -fno-exceptions

AS = arm-none-eabi-as
ASFLAGS = -mcpu=arm926ej-s


SOURCE_DIR = source
OBJ_DIR = obj
BIN_DIR = bin
TOOLS_DIR = tools

OBJECTS =	$(OBJ_DIR)\io_angel.o					\
			$(OBJ_DIR)\io_serial.o 					\
			$(OBJ_DIR)\keyboard_mouse_interface.o 	\
			$(OBJ_DIR)\cpu.o 						\
			$(OBJ_DIR)\timer.o						\
			$(OBJ_DIR)\pic.o 						\
			$(OBJ_DIR)\stack.o 						\
			$(OBJ_DIR)\device_driver.o

all : $(BIN_DIR)\dump_cpu_state.elf $(BIN_DIR)\atose.elf

#
# ATOSE
#
$(BIN_DIR)\atose.elf : $(SOURCE_DIR)\atose.c startup.o $(OBJECTS) $(SOURCE_DIR)\atose.ld
	$(CC) $(CFLAGS) -o $(BIN_DIR)\atose.elf $(SOURCE_DIR)\atose.c $(OBJECTS) startup.o -T $(SOURCE_DIR)\atose.ld

startup.o : $(SOURCE_DIR)\atose_startup.asm
	$(AS) $(ASFLAGS) $(SOURCE_DIR)\atose_startup.asm -o startup.o

#
# ATOSE Tools
#
$(BIN_DIR)\dump_cpu_state.elf : $(TOOLS_DIR)\dump_cpu_state.c
	$(CC) -o $(BIN_DIR)\dump_cpu_state.elf $(TOOLS_DIR)\dump_cpu_state.c -T generic-hosted.ld

#
# Management
#
run:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel $(BIN_DIR)\atose.elf -serial stdio

qemu:
	"\Program Files\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel $(BIN_DIR)\atose.elf -serial stdio

clean:
	del *.bak *.elf *.o /s

#
# Implicit rules
#
{$(SOURCE_DIR)\}.c{$(OBJ_DIR)\}.o:
	$(CC) $(CFLAGS) -c $< -o $@

