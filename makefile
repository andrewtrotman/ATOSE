#
# Makefile for ATOSE and the associated tools
#
CC = arm-none-eabi-g++
CFLAGS = -g -mcpu=arm926ej-s

OBJ = io_angel.o	\
		mmu.o		\
		interrupts.o	\
		main.o

all : dump_cpu_state.elf timer.elf atose.elf test.elf

#
#	ATOSE itself
#
atose.elf : $(OBJ)
	$(CC) -o atose.elf $** -T generic-hosted.ld

#
# ATOSE Tools
#
dump_cpu_state.elf : dump_cpu_state.c
	$(CC) -o dump_cpu_state.elf dump_cpu_state.c -T generic-hosted.ld

timer.elf : timer.c
	$(CC) -o timer.elf timer.c -T generic-hosted.ld

test.elf : test.c vectors.s test.ld
	$(CC) $(CFLAGS) -c test.c -o test.o
	$(CC) $(CFLAGS) -c vectors.s -o vectors.o
	$(CC) $(CFLAGS) -T test.ld test.o vectors.o -o test.elf

#
# Management
#
run:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel timer.elf

qemu:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -kernel atose.elf

clean:
	del *.bak *.elf *.o

#
# Implicit rules
#
.c.o:
	$(CC) $(CFLAGS) -c -g $<

