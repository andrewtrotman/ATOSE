/*
	ELF.C
	-----
*/
#include "elf.h"

/*
	MEM_RANGE
	---------
	A range of memory, used to validate the ELF file
*/
typedef struct
{
long long start;
long long finish;
} MEM_range;

MEM_range range[1024];		// we allow 1024 program segments
long range_current = 0;

/*
	VALIDATE()
	----------
*/
inline void validate(long long address)
{
long which_range;

for (which_range = 0; which_range < range_current; which_range++)
	if (address >= range[which_range].start && address <= range[which_range].finish)
		return;

printf("NOT IN RANGE: %X\n", address);
}


/*
	READ_ENTIRE_FILE()
	------------------
*/
char *read_entire_file(char *filename, long long *file_length)
{
long long unused;
char *block = NULL;
FILE *fp;
struct stat details;

if (filename == NULL)
	return NULL;

if (file_length == NULL)
	file_length = &unused;

if ((fp = fopen(filename, "rb")) == NULL)
	return NULL;

if (fstat(fileno(fp), &details) == 0)
	if ((*file_length = details.st_size) != 0)
		if ((block = new (std::nothrow) char [(size_t)(details.st_size + 1)]) != NULL)		// +1 for the '\0' on the end
			if (fread(block, (long)details.st_size, 1, fp) == 1)
				block[details.st_size] = '\0';
			else
				{
				delete [] block;
				block = NULL;
				}
fclose(fp);

return block;
}

/*
	READ_PROGRAM_HEADER()
	---------------------
*/
void read_program_header(char *file, long long file_size, Elf32_Ehdr *ELF_header)
{
uint32_t header_offset;		// location (in the file) of the first header
uint32_t header_size;			// size of each header
uint32_t header_num;			// number of headers
Elf32_Phdr *current_header;	// the current header
long which;					// which header we're currenty looking at

printf("ELF 32 program headers\n");
printf("======================\n");

header_offset = ELF_header->e_phoff;
header_size = ELF_header->e_phentsize;
header_num = ELF_header->e_phnum;

if (header_size != sizeof(Elf32_Phdr))
	exit(printf("Program headers are too long\n"));
	
if ((header_offset + header_size *header_num) > file_size)
	exit(printf("Program headers located past end of file\n"));

current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	if (which != 0)
		printf("\n");
	printf("p_type             : %u (0x%08X)\n", current_header->p_type, current_header->p_type);
	printf("p_offset           : %u\n", current_header->p_offset);
	printf("p_vaddr            : 0x%08x\n", current_header->p_vaddr);
	printf("   uses            : 0x%08x - 0x%08x\n", current_header->p_vaddr, current_header->p_vaddr + current_header->p_memsz);

	range[range_current].start = current_header->p_vaddr;
	range[range_current].finish = current_header->p_vaddr + current_header->p_memsz;
	range_current++;

	printf("p_paddr            : 0x%08x\n", current_header->p_paddr);
	printf("p_filesz           : %u\n", current_header->p_filesz);
	printf("p_memsz            : %u\n", current_header->p_memsz);
	printf("p_flags            : %u (", current_header->p_flags);
	if (current_header->p_flags & PF_R)
		printf("R");
	if (current_header->p_flags & PF_W)
		printf("W");
	if (current_header->p_flags & PF_X)
		printf("X");
	printf(")\n");

	printf("p_align            : %u\n", current_header->p_align);

	/*
		Now check it
	*/
	if (current_header->p_type != PT_ARM_UNWIND && current_header->p_type != PT_LOAD)
		exit(printf("Unknown program segment type:0x%08x\n",  current_header->p_type));

	/*
		Move on to the next segment
	*/	
	current_header++;
	}
}

/*
	READ_SECTION_HEADER()
	---------------------
*/
void read_section_header(char *file, long long file_size, Elf32_Ehdr *ELF_header)
{
uint32_t header_offset;		// location (in the file) of the first header
uint32_t header_size;			// size of each header
uint32_t header_num;			// number of headers
char *header_strings;			// the offset into the file of the string table
Elf32_Shdr *current_header;	// the current header
long which;					// which header we're currenty looking at
long which_byte;

printf("ELF 32 section headers\n");
printf("======================\n");

header_offset = ELF_header->e_shoff;
header_size = ELF_header->e_shentsize;
header_num = ELF_header->e_shnum;
header_strings = file + ((Elf32_Shdr *)(file + header_offset))[ELF_header->e_shstrndx].sh_offset;

if (header_size != sizeof(Elf32_Shdr))
	exit(printf("Section headers are too long\n"));
	
if ((header_offset + header_size *header_num) > file_size)
	exit(printf("Section headers located past end of file\n"));

current_header = (Elf32_Shdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	if (which != 0)
		printf("\n");

	printf("sh_name            : %s\n", header_strings + current_header->sh_name);
	printf("sh_type            : %u (%08X)\n", current_header->sh_type, current_header->sh_type);
	printf("sh_flags           : %02X ", current_header->sh_flags);

	if (current_header->sh_flags & SHF_ALLOC)
		printf("R");
	if (current_header->sh_flags & SHF_EXECINSTR)
		printf("X");
	if (current_header->sh_flags & SHF_WRITE)
		printf("W");
	printf("\n");

	printf("sh_addr            : %u (0x%08X)\n", current_header->sh_addr, current_header->sh_addr);
	printf("sh_offset          : %u\n", current_header->sh_offset);
	printf("sh_size            : %u\n", current_header->sh_size);
	printf("sh_link            : %u\n", current_header->sh_link);
	printf("sh_info            : %u\n", current_header->sh_info);
	printf("sh_addralign       : %u\n", current_header->sh_addralign);
	printf("sh_entsize         : %u\n", current_header->sh_entsize);

	/*
		validate the ones we allocate space to
	*/
	if (current_header->sh_flags & SHF_ALLOC)
		for (which_byte = current_header->sh_addr; which_byte < current_header->sh_addr + current_header->sh_size; which_byte++)
			validate(which_byte);
	/*
		Move on to the next segment
	*/	
	current_header++;
	}
}

/*
	ATOSE_ELF::LOAD()
	-----------------
	We need to check as much of this as we can because this is an obvious way
	to attack the kernel.

	Return 0 on success all other codes are errors (see elf_errors.h)
*/
uint32_t ATOSE_elf::load(uint8_t *file, uint32_t length)
{
Elf32_Ehdr *header;
long header_ok, current;

/*
	An ELF file must be at least the length of the header.
*/
if (length < sizeof(Elf32_Ehdr))
	return ELF_BAD_FROM_START;		// the header is broken

header = (Elf32_Ehdr *)file;

/*
	First verify that we have and ELF file
*/
header_ok = 0;
header_ok += (file[EI_MAG0] == 0x7F);
header_ok += (file[EI_MAG1] == 'E');
header_ok += (file[EI_MAG2] == 'L');
header_ok += (file[EI_MAG3] == 'F');

if (header_ok != 4)
	return ELF_BAD_ID;

/*
	Check we have a 32-bit file
*/
if (file[EI_CLASS] != ELFCLASS32)
	return ELF_BAD_BITNESS;

/*
	Check we have a little endian file
*/
if (file[EI_DATA] != ELFDATA2LSB)
	return ELF_BAD_ENDIAN;

/*
	Check we have a file version we understand
*/
if (file[EI_VERSION] != EV_CURRENT)
	return ELF_BAD_VERSION;

/*
	Make sure the header is the right length
*/
if (header->e_ehsize != sizeof(Elf32_Ehdr))
	return ELF_BAD_HEADER;

/*
	Make sure the header is a version we understand
*/
if (header->e_version != EV_CURRENT)
	return ELF_BAD_HEADER_VERSION;

/*
	Now check we're an executable file
*/
if (header->e_type != ET_EXEC)
	return ELF_BAD_TYPE;

/*
	Check we're an ARM executable file
*/
if (header->e_machine != EM_ARM)
	return ELF_BAD_ARCHITECTURE;

/*
	We passed the test so at this point we think we have an ELF
	file thats ARM 32-bit little endian
*/
if (header->e_phnum != 0)
	{
	printf("\n");
	read_program_header(file, length, header);
	}

if (header->e_shnum != 0)
	{
	printf("\n");
	read_section_header(file, length, header);
	}

return 0;
}
