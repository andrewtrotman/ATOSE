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
CFLAGS = -std=c++0x -mcpu=cortex-a9 -D$(CPU) -D$(TARGET) -g3 -Wall -Os -fno-exceptions -fno-rtti -ffreestanding -nostdlib -nodefaultlibs -nostartfiles
CLINKFLAGS = -l gcc

AS = @arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a9


#
# WARNING-> atose_startup.o *must* be the first object in the list so that imx_run.exe can load and run the program
#
OBJECTS =						\
	atose_startup.o				\
	atose_api.o						\
	address_space.o				\
	atose.o						\
	clock_imx6q.o				\
	cpu_arm.o					\
	cpu_arm_imx6q.o				\
	ctypes.o				\
	debug.o						\
	fat.o							\
	file_system.o				\
	host_usb.o					\
	host_usb_device.o			\
	host_usb_device_disk.o		\
	host_usb_device_hub.o		\
	idle.o						\
	interrupt.o					\
	interrupt_arm_gic.o			\
	kernel_malloc.o				\
	kernel_memory_allocator.o	\
	main.o						\
	mmu.o						\
	mmu_page_list.o				\
	pipe.o						\
	pipe_test.o					\
	process_manager.o			\
	process_allocator.o			\
	semaphore.o					\
	sleep_wake.o				\
	stack.o						\
	timer_imx6q.o				\
	uart_imx6q.o				\
	usb.o						\
	usb_imx6q.o



all : atose.elf

atose.elf : $(OBJECTS) imx6q.ld examples\hello.elf.c
	@echo $@
	$(CC) $(CFLAGS) -o atose.elf $(OBJECTS) -T imx6q.ld $(CLINKFLAGS)

examples\hello.elf.c : examples\hello.c
	@cd examples
	@make /nologo
	@cd ..

atose.o : examples\hello.elf.c

clean :
	@del $(OBJECTS) atose.elf
	@cd examples
	@make /nologo clean
	@cd ..

.c.o:
	@echo $@
	$(CC) $(CFLAGS) -c $< -o $@

.asm.o:
	@echo $@
	$(AS) $(ASFLAGS) $< -o $@

