/*
	ELF_READER.C
	------------
*/

#define EINIDENT 16

#include <stdint.h>

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef uint32_t	Elf32_Addr;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Off;
typedef int32_t	Elf32_Sword;
typedef uint32_t	Elf32_Word;
typedef uint32_t	Elf32_Size;

#define EI_NIDENT	16	/* Size of e_ident array. */

/*
	ELF32_EHDR
	----------
	ELF file header
*/
typedef struct
{
unsigned char	e_ident[EI_NIDENT];	/* File identification. */
Elf32_Half	e_type;
Elf32_Half	e_machine;
Elf32_Word	e_version;
Elf32_Addr	e_entry;
Elf32_Off	e_phoff;
Elf32_Off	e_shoff;
Elf32_Word	e_flags;
Elf32_Half	e_ehsize;
Elf32_Half	e_phentsize;
Elf32_Half	e_phnum;
Elf32_Half	e_shentsize;
Elf32_Half	e_shnum;
Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

/*
	e_ident (file offsets of the magic numbers necessary to identify and ELF file)
*/
#define EI_MAG0 0
#define EI_MAG1 1
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
#define EI_VERSION 6
#define EI_PAD 7

/*
	the EI_CLASS member of e_ident - we only care about 32-bit files
*/
#define ELFCLASS32 1

/*
	the EI_DATA member of e_ident - we only care about little endian files
*/
#define ELFDATA2LSB 1

/*
	e_type values (we only care about EXE files
*/
#define ET_EXEC 2

/*
	e_machine values (we only care about ARM)
*/
#define EM_ARM 40

/*
	e_version - the version number if the ELF file format
*/
#define EV_CURRENT 1

/*
	ELF32_PHDR
	----------
	ELF program header
*/
typedef struct
{
Elf32_Word	p_type;
Elf32_Off	p_offset;
Elf32_Addr	p_vaddr;
Elf32_Addr	p_paddr;
Elf32_Size	p_filesz;
Elf32_Size	p_memsz;
Elf32_Word	p_flags;
Elf32_Size	p_align;
} Elf32_Phdr;

/*
	p_type values (we only care about what EXE relevant ones)
*/
#define PT_ARM_UNWIND 0x70000001
#define PT_LOAD 1

/*
	p_flags (section flags)
*/
#define PF_X 0x01	// execute permitted
#define PF_W 0x02	// write permitted
#define PF_R 0x04	// read permitted

/*
	ELF32_SHDR
	----------
	ELF section header
*/
typedef struct
{
Elf32_Word	sh_name;
Elf32_Word	sh_type;
Elf32_Word	sh_flags;
Elf32_Addr	sh_addr;
Elf32_Off	sh_offset;
Elf32_Size	sh_size;
Elf32_Word	sh_link;
Elf32_Word	sh_info;
Elf32_Size	sh_addralign;
Elf32_Size	sh_entsize;
} Elf32_Shdr;

/*
	sh_flags (section flags)
*/
#define SHF_WRITE 0x01
#define SHF_ALLOC 0x02
#define SHF_EXECINSTR 0x04



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
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
char *file;
long long length;
Elf32_Ehdr *header;
long header_ok, current;

/*
	Check my parameters
*/
if (argc != 2)
	exit(printf("Usage:%s <elf_file>\n", argv[0]));

/*
	read the ELF file into memory
*/
if ((file = read_entire_file(argv[1], &length)) == NULL)
	exit(printf("Cannot open elf file:%s\n", argv[1]));

/*
	Verify the file read
*/
if (length < sizeof(Elf32_Ehdr))
	exit(printf("Invalid ELF file (too small)\n"));

header = (Elf32_Ehdr *)file;

/*
	First verify that we have and ELF file that we can understand
*/
header_ok = 0;
header_ok += (file[EI_MAG0] == 0x7F);
header_ok += (file[EI_MAG1] == 'E');
header_ok += (file[EI_MAG2] == 'L');
header_ok += (file[EI_MAG3] == 'F');

if (header_ok != 4)
	exit(printf("Not and ELF file (the ELF magic number is missing)\n"));

if (file[EI_CLASS] != ELFCLASS32)
	exit(printf("Not a 32 bit file\n"));

if (file[EI_DATA] != ELFDATA2LSB)
	exit(printf("Not a little endian file\n"));

if (file[EI_VERSION] != EV_CURRENT)
	exit(printf("Unknown verison of the ELF format\n"));

if (header->e_ehsize != sizeof(Elf32_Ehdr))
	exit(printf("The ELF is the wrong size (is this and ELF file?\n"));

if (header->e_version != EV_CURRENT)
	exit(printf("Unknown verison of the ELF format\n"));

/*
	Now print out the details of the file
*/
printf("ELF 32 file header\n");
printf("==================\n");
printf("e_ident[EI_NIDENT] : ");
for (current = 0; current < EI_NIDENT; current++)
	printf("%02X ", header->e_ident[current]);
for (current = 0; current < EI_NIDENT; current++)
	printf("%c ", isprint(header->e_ident[current]) ? header->e_ident[current] : '.');
printf("\n");
printf("e_type             : %u\n", header->e_type);
printf("e_machine          : %u\n", header->e_machine);
printf("e_version          : %u\n", header->e_version);
printf("e_entry            : %u (%08X)\n", header->e_entry, header->e_entry);
printf("e_phoff            : %u\n", header->e_phoff);
printf("e_shoff            : %u\n", header->e_shoff);
printf("e_flags            : %u\n", header->e_flags);
printf("e_ehsize           : %u\n", header->e_ehsize);
printf("e_phentsize        : %u\n", header->e_phentsize);
printf("e_phnum            : %u\n", header->e_phnum);
printf("e_shentsize        : %u\n", header->e_shentsize);
printf("e_shnum            : %u\n", header->e_shnum);
printf("e_shstrndx         : %u\n", header->e_shstrndx);

/*
	Now check we're and ARM 32 EXE file
*/
//if (header->e_type != ET_EXEC)
//	exit(printf("File is not an executable\n"));

if (header->e_machine != EM_ARM)
	exit(printf("File is not for ARM\n"));

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