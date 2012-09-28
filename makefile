#
# Makefile for ATOSE and the associated tools
#

#
#	Choose a Target for ATOSE
#
#
TARGET = FourARM
#
#TARGET = QEMU
#

!IF "$(TARGET)" == "FourARM"
CPU = IMX233
!ELSE
CPU = ARM926
!ENDIF

CC = arm-none-eabi-g++
CCC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm926ej-s -ffreestanding -fno-exceptions -fno-rtti -nostdlib -nodefaultlibs -nostartfiles -D$(CPU) -D$(TARGET) -Os -g3
CLINKFLAGS = -l gcc

AS = arm-none-eabi-as
ASFLAGS = -mcpu=arm926ej-s

SOURCE_DIR = source
OBJ_DIR = obj
BIN_DIR = bin
TOOLS_DIR = tools
TESTS_DIR = tests

ATOSE_OBJECTS =									\
			$(OBJ_DIR)\cpu.o 						\
			$(OBJ_DIR)\stack.o 						\
			$(OBJ_DIR)\atose.o						\
			$(OBJ_DIR)\device_driver.o				\
			$(OBJ_DIR)\interrupts.o					\
			$(OBJ_DIR)\kernel_memory_allocator.o	\
			$(OBJ_DIR)\nand.o

FourARM_OBJECTS =									\
			$(OBJ_DIR)\io_debug_imx233.o 			\
			$(OBJ_DIR)\timer_imx233.o				\
			$(OBJ_DIR)\pic_imx233.o					\
			$(OBJ_DIR)\nand_imx233.o

QEMU_OBJECTS =									\
			$(OBJ_DIR)\io_angel.o					\
			$(OBJ_DIR)\io_serial.o 					\
			$(OBJ_DIR)\keyboard_mouse_interface.o 	\
			$(OBJ_DIR)\timer_sp804.o				\
			$(OBJ_DIR)\pic_pl190.o


!IF "$(TARGET)" == "FourARM"

OBJECTS = $(ATOSE_OBJECTS) $(FourARM_OBJECTS)

!ELSE

OBJECTS = $(ATOSE_OBJECTS) $(QEMU_OBJECTS)

!ENDIF


all : $(BIN_DIR)\dump_cpu_state.elf $(BIN_DIR)\atose.elf $(BIN_DIR)\elf_reader.exe $(BIN_DIR)\imx233_timer.elf $(BIN_DIR)\imx233_nand.elf

#
# ATOSE
#
$(BIN_DIR)\atose.elf : startup.o $(OBJ_DIR)\main.o $(OBJECTS) $(SOURCE_DIR)\atose.ld
	$(CC) $(CFLAGS) -o $(BIN_DIR)\atose.elf startup.o $(OBJ_DIR)\main.o $(OBJECTS) -T $(SOURCE_DIR)\atose.ld $(CLINKFLAGS)

startup.o : $(SOURCE_DIR)\atose_startup.asm
	$(AS) $(ASFLAGS) $(SOURCE_DIR)\atose_startup.asm -o startup.o

#
# ATOSE Tools
#
$(BIN_DIR)\imx233_timer.elf : $(TESTS_DIR)\imx233_timer.c startup.o $(SOURCE_DIR)\atose.ld
	$(CCC) -Os -o $(BIN_DIR)\imx233_timer.elf startup.o $(TESTS_DIR)\imx233_timer.c -T $(SOURCE_DIR)\atose.ld

$(BIN_DIR)\imx233_nand.elf : $(TESTS_DIR)\imx233_nand.c startup.o $(SOURCE_DIR)\atose.ld
	$(CCC) -Os -o $(BIN_DIR)\imx233_nand.elf startup.o $(TESTS_DIR)\imx233_nand.c -T $(SOURCE_DIR)\atose.ld

$(BIN_DIR)\dump_cpu_state.elf : $(TOOLS_DIR)\dump_cpu_state.c startup.o $(SOURCE_DIR)\atose.ld
	$(CCC) -Os -o $(BIN_DIR)\dump_cpu_state.elf startup.o $(TOOLS_DIR)\dump_cpu_state.c -T $(SOURCE_DIR)\atose.ld

$(BIN_DIR)\elf_reader.exe : $(TOOLS_DIR)\elf_reader.c
#	cl /Tp $(TOOLS_DIR)\elf_reader.c -Fe$(BIN_DIR)\elf_reader.exe


#
# Management
#
run:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -cpu arm926 -kernel $(BIN_DIR)\atose.elf -serial stdio

qemu:
	"\Program Files\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel $(BIN_DIR)\atose.elf -serial stdio

clean:
	del bin obj startup.o /q

#
# Implicit rules
#
{$(SOURCE_DIR)\}.c{$(OBJ_DIR)\}.o:
	$(CC) $(CFLAGS) -c $< -o $@

