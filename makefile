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

#
#	Which version of the Window DDK do we have?
#
VER_8 = "C:\Program Files (x86)\Windows Kits\8.0\"
!IF "$(WINDOWSSDKDIR)" == $(VER_8)
DDK_DIR = $(WINDOWSSDKDIR)\Include
DDK_INCLUDE = "$(DDK_DIR)\shared" -I"$(DDK_DIR)\km" -I"$(DDK_DIR)\km\crt" -I"$(DDK_DIR)\um"
DDK_LIB = "$(WINDOWSSDKDIR)\Lib\win8\um\x64\setupapi.lib" "$(WINDOWSSDKDIR)\Lib\win8\um\x64\hid.lib"
!ELSE
DDK_DIR = "c:\WinDDK\7600.16385.1\inc"
DDK_INCLUDE = $(DDK_DIR)\ddk -I$(DDK_DIR)\api -I$(DDK_DIR)\crt
DDK_LIB = c:\WinDDK\7600.16385.1\lib\wxp\i386\setupapi.lib c:\WinDDK\7600.16385.1\lib\wxp\i386\hid.lib
!ENDIF



#
#	Now the compilers, etc.
#
CCC = @arm-none-eabi-gcc
CCCFLAGS = -mcpu=arm926ej-s  -D$(CPU) -D$(TARGET) -g3 -Wall -Os
CC = @arm-none-eabi-g++
CFLAGS = -fno-exceptions -fno-rtti $(CCCFLAGS) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles
CLINKFLAGS = -l gcc

AS = @arm-none-eabi-as
ASFLAGS = -mcpu=arm926ej-s

SOURCE_DIR = source
OBJ_DIR = obj
BIN_DIR = bin
TOOLS_DIR = tools
TESTS_DIR = tests
EXAMPLES_DIR = examples

#
#	The core of ATOSE Kernel
#
ATOSE_OBJECTS =								\
	$(OBJ_DIR)\address_space.o				\
	$(OBJ_DIR)\atose.o						\
	$(OBJ_DIR)\cpu.o 						\
	$(OBJ_DIR)\device_driver.o				\
	$(OBJ_DIR)\interrupts.o					\
	$(OBJ_DIR)\kernel_memory_allocator.o	\
	$(OBJ_DIR)\mmu.o 						\
	$(OBJ_DIR)\mmu_page_list.o				\
	$(OBJ_DIR)\process_manager.o			\
	$(OBJ_DIR)\stack.o

#
#	FourARM specific parts of the ATOSE Kernel
#
FourARM_OBJECTS =							\
	$(OBJ_DIR)\io_debug_imx233.o 			\
	$(OBJ_DIR)\nand.o						\
	$(OBJ_DIR)\nand_imx233.o				\
	$(OBJ_DIR)\pic_imx233.o					\
	$(OBJ_DIR)\timer_imx233.o

#
#	QEMU (Versatile) specific parts of the ATOSE Kernel
#
QEMU_OBJECTS =								\
	$(OBJ_DIR)\io_angel.o					\
	$(OBJ_DIR)\io_serial.o 					\
	$(OBJ_DIR)\keyboard_mouse_interface.o 	\
	$(OBJ_DIR)\pic_pl190.o					\
	$(OBJ_DIR)\timer_sp804.o

#
#	Useful stuff written to test various components of various devices
#
ATOSE_TOOLS =								\
	$(BIN_DIR)\dump_cpu_state.elf			\
	$(BIN_DIR)\imx233_nand.elf				\
	$(BIN_DIR)\imx233_mmu.elf				\
	$(BIN_DIR)\imx233_timer.elf				\
	$(BIN_DIR)\imx233_usb.elf				\
	$(BIN_DIR)\test_ram.elf

IMX6Q_TOOLS =						\
	$(BIN_DIR)\imx6q_uart.elf

#
#	Collect the set of objects needed for the Kernel (based on the Target)
#
!IF "$(TARGET)" == "FourARM"

OBJECTS = $(ATOSE_OBJECTS) $(FourARM_OBJECTS)

!ELSE

OBJECTS = $(ATOSE_OBJECTS) $(QEMU_OBJECTS)

!ENDIF


this : $(EXAMPLES_DIR)\hello.elf.c all

all : 									\
	$(BIN_DIR)\atose.elf 				\
	$(BIN_DIR)\bin_to_c.exe				\
	$(BIN_DIR)\imx_run.exe              \
	$(BIN_DIR)\imximage_render.exe      \
	$(BIN_DIR)\elf_reader.exe 			\
	$(BIN_DIR)\hello.elf				\
	$(ATOSE_TOOLS)						\
	$(IMX6Q_TOOLS)

$(ATOSE_TOOLS) : startup.o $(SOURCE_DIR)\atose.ld

$(BIN_DIR)\imx6q_uart.elf : $(TESTS_DIR)\imx6q.ld $(TESTS_DIR)\imx6q.s
	@echo $@
	$(CCC) $(CCCFLAGS) $(TESTS_DIR)\imx6q_uart.c $(TESTS_DIR)\imx6q.s $(CLINKFLAGS) -o $@  -T $(TESTS_DIR)\imx6q.ld -I windows_HID\hello\

#
# ATOSE
#
$(BIN_DIR)\atose.elf : startup.o $(OBJ_DIR)\main.o $(OBJECTS) $(SOURCE_DIR)\atose.ld
	@echo $@
	$(CC) $(CFLAGS) -o $(BIN_DIR)\atose.elf startup.o $(OBJ_DIR)\main.o $(OBJECTS) -T $(SOURCE_DIR)\atose.ld $(CLINKFLAGS)
	@arm-none-eabi-objdump -S -d bin\atose.elf > ers

startup.o : $(SOURCE_DIR)\atose_startup.asm
	@echo $@
	$(AS) $(ASFLAGS) $(SOURCE_DIR)\atose_startup.asm -o startup.o

#
#	ATOSE programs (stuff that runs within ATOSE itself
#
$(OBJ_DIR)\atose_process_entry_point.o : $(EXAMPLES_DIR)\atose_process_entry_point.asm
	@echo $@
	$(AS) $(ASFLAGS) $(EXAMPLES_DIR)\atose_process_entry_point.asm -o $(OBJ_DIR)\atose_process_entry_point.o

$(BIN_DIR)\hello.elf : $(EXAMPLES_DIR)\hello.c $(EXAMPLES_DIR)\atose_process.ld $(OBJ_DIR)\atose_process_entry_point.o  $(BIN_DIR)\bin_to_c.exe
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -g0 -fno-exceptions -fno-rtti -ffreestanding -Xlinker -z -Xlinker max-page-size=0x0100 -o $(BIN_DIR)\hello.elf -I source $(EXAMPLES_DIR)\hello.c -T $(EXAMPLES_DIR)\atose_process.ld $(CLINKFLAGS)

$(EXAMPLES_DIR)\hello.elf.c hello : $(BIN_DIR)\hello.elf
	@arm-none-eabi-strip --strip-all --remove-section=.comment --remove-section=.note $(BIN_DIR)\hello.elf
	@$(BIN_DIR)\bin_to_c.exe $(BIN_DIR)\hello.elf $(EXAMPLES_DIR)\hello.elf.c ATOSE_elf_hello

#
# Management
#
run:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -cpu arm926 -kernel $(BIN_DIR)\atose.elf -serial stdio -m 256M

debug:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -cpu arm926 -kernel $(BIN_DIR)\atose.elf -serial stdio -m 256M -s -gdb tcp::1234,ipv4


qemu:
	"\Program Files\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel $(BIN_DIR)\atose.elf -serial stdio -m 256M

clean:
	del bin obj startup.o $(EXAMPLES_DIR)\hello.elf.c /q

#
#	Special rules
#

$(BIN_DIR)\imx_run.exe : $(TOOLS_DIR)\imx_run.c
	@cl /nologo /W3 /Zi /Tp $(TOOLS_DIR)\imx_run.c /X -I$(DDK_INCLUDE) $(DDK_LIB) -Fe$(BIN_DIR)\imx_run.exe -Fo$(OBJ_DIR)\$(@B).obj

#
# Implicit rules
#
{$(SOURCE_DIR)\}.c{$(OBJ_DIR)\}.o:
	@echo $@
	$(CC) $(CFLAGS) -c $< -o $@

{$(TOOLS_DIR)\}.c{$(BIN_DIR)\}.exe:
	@echo $@
	@cl /nologo /Tp $< -Fe$@ -Fo$(OBJ_DIR)\$(@B).obj -D_CRT_SECURE_NO_DEPRECATE
	
{$(TESTS_DIR)}.c{$(BIN_DIR)}.elf:
	@echo $@
	$(CCC) $(CCCFLAGS) $< $(CLINKFLAGS) -o $@ startup.o -T $(SOURCE_DIR)\atose.ld
