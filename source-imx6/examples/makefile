
CC = @arm-none-eabi-g++
CFLAGS = -fno-exceptions -fno-rtti $(CCCFLAGS) -ffreestanding -nostdlib -nodefaultlibs -nostartfiles
CLINKFLAGS = -l gcc

AS = @arm-none-eabi-as
ASFLAGS = -mcpu=arm926ej-s


all : hello1.elf hello2.elf

hello1.elf : hello.c atose_process.ld atose_process_entry_point.o
	$(CC) $(CFLAGS) -DCHARACTER='B' -ffunction-sections -fdata-sections -g0 -fno-exceptions -fno-rtti -ffreestanding -Xlinker -z -Xlinker max-page-size=0x0100 -o hello1.elf hello.c -T atose_process.ld $(CLINKFLAGS)
	@..\..\bin\bin_to_c hello1.elf hello1.elf.c hello1_elf

hello2.elf : hello.c atose_process.ld atose_process_entry_point.o
	$(CC) $(CFLAGS) -DCHARACTER='C' -ffunction-sections -fdata-sections -g0 -fno-exceptions -fno-rtti -ffreestanding -Xlinker -z -Xlinker max-page-size=0x0100 -o hello2.elf hello.c -T atose_process.ld $(CLINKFLAGS)
	@..\..\bin\bin_to_c hello2.elf hello2.elf.c hello2_elf

atose_process_entry_point.o : atose_process_entry_point.asm
	@echo $@
	$(AS) $(ASFLAGS) atose_process_entry_point.asm -o atose_process_entry_point.o


clean :
	@del hello.elf hello.elf.c hello.o atose_process_entry_point.o
