
CC = @arm-none-eabi-g++
CFLAGS = -fno-exceptions -fno-rtti $(CCCFLAGS) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles
CLINKFLAGS = -l gcc

AS = @arm-none-eabi-as
ASFLAGS = -mcpu=arm926ej-s


all : shell.elf

shell.elf : shell.c atose_process.ld atose_process_entry_point.o
	$(CC) $(CFLAGS) -ffunction-sections -fdata-sections -g0 -fno-exceptions -fno-rtti -ffreestanding -Xlinker -z -Xlinker max-page-size=0x0100 -o shell.elf shell.c -T atose_process.ld $(CLINKFLAGS)

atose_process_entry_point.o : atose_process_entry_point.asm
	@echo $@
	$(AS) $(ASFLAGS) atose_process_entry_point.asm -o atose_process_entry_point.o

clean :
	rm shell.elf atose_process_entry_point.o
