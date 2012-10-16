/*
	ELF.H
	-----
	This header file includes the declarations of all the structures necessary
	to load and execute an EFL file
*/
#ifndef ELF_H_
#define ELF_H_

#include <stdint.h>

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

#endif /* ELF_H_ */
