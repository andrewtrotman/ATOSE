CC = arm-none-eabi-g++
#CFLAGS = -mcpu=cortex-a9 -ffreestanding -nostartfiles -fno-exceptions -fno-rtti -nostdlib -nodefaultlibs -lm -lgcc
CFLAGS = -mcpu=cortex-a9 -ffreestanding -nostartfiles -fno-exceptions -fno-rtti 

AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a9


all : shell.elf

shell.elf : shell.c atose_process.ld atose_process_entry_point.o 
	$(CC) $(CFLAGS) -o shell.elf shell.c atose_process_entry_point.o ../obj/atose_api.o ../obj/ctypes.o -T armelf.ld -Xlinker  -z -Xlinker max-page-size=0x100000 -Xlinker -z -Xlinker common-page-size=0x100000
#	$(CC) $(CFLAGS) -Xlinker -z -Xlinker max-page-size=0x0100 -o shell.elf shell.c ../obj/atose_api.o ../obj/ctypes.o -T atose_process.ld $(CFLAGS)

atose_process_entry_point.o : atose_process_entry_point.asm
	@echo $@
	$(AS) $(ASFLAGS) atose_process_entry_point.asm -o atose_process_entry_point.o

clean :
	rm shell.elf atose_process_entry_point.o
