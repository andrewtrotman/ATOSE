#
#	Copyright (c) 2013 Andrew Trotman
#	Licensed BSD
#

#
# Possible values for TARGET are:
#	SABRE_LITE: The boundaryDevice's SABRE Lite board
#	Octopus   : Our Octopus board
#
TARGET = SABRE_LITE
CPU = IMX6Q

CC = @arm-none-eabi-g++
CFLAGS = -mcpu=cortex-a9 -D$(CPU) -D$(TARGET) -g3 -Wall -Os -fno-exceptions -fno-rtti -ffreestanding -nostdlib -nodefaultlibs -nostartfiles
CLINKFLAGS = -l gcc

AS = @arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a9

OBJECTS =						\
	atose_startup.o				\
	atose.o						\
	clock_imx6q.o				\
	cpu_arm.o					\
	cpu_arm_imx6q.o				\
	debug.o						\
	host_usb.o					\
	interrupt.o					\
	interrupt_arm_gic.o			\
	kernel_malloc.o					\
	main.o						\
	stack.o						\
	timer_imx6q.o				\
	uart_imx6q.o				\
	usb.o						\
	usb_imx6q.o

all : atose.elf

atose.elf : $(OBJECTS) imx6q.ld
	@echo $@
	$(CC) $(CFLAGS) -o atose.elf $(OBJECTS) -T imx6q.ld $(CLINKFLAGS)

clean :
	del $(OBJECTS) atose.elf

.c.o:
	@echo $@
	$(CC) $(CFLAGS) -c $< -o $@

.asm.o:
	@echo $@
	$(AS) $(ASFLAGS) $< -o $@

