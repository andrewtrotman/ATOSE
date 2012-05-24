#
# Makefile for ATOSE and the associated tools
#
CC = arm-none-eabi-g++
CFLAGS = -g -mcpu=arm926ej-s

OBJ = io_angel.o		\
		io_serial.o 	\
		mmu.o			\
		interrupts.o	\
		main.o

all : dump_cpu_state.elf timer.elf atose.elf test.elf timerII.elf

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

timerII.elf : timerII.c
	$(CC) -o timerII.elf timerII.c io_angel.c io_serial.c -T generic-hosted.ld

test.elf : test.c vectors.s test.ld
	$(CC) $(CFLAGS) -c test.c -o test.o -nostartfiles -nodefaultlibs -fno-rtti 
	$(CC) $(CFLAGS) -c vectors.s -o vectors.o -nostartfiles -nodefaultlibs -fno-rtti 
	$(CC) $(CFLAGS) -c heap.c -o heap.o -nostartfiles -nodefaultlibs -fno-rtti 
	$(CC) $(CFLAGS) -T test.ld test.o vectors.o heap.o -o test.elf -nostartfiles -nodefaultlibs -fno-rtti 

#
# Management
#
run:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -M versatileab -kernel timerII.elf -serial stdio 

qemu:
	"\Program Files (x86)\qemu\qemu-system-arm.exe" -semihosting -kernel atose.elf

clean:
	del *.bak *.elf *.o

#
# Implicit rules
#
.c.o:
	$(CC) $(CFLAGS) -c -g $<

