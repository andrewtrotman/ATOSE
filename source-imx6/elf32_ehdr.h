/*
	ELF32_EHDR.H
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This header file includes the declarations of all the structures necessary
	to load and execute an EFL file
*/
#ifndef ELF32_EHDR_H_
#define ELF32_EHDR_H_

#include <stdint.h>

/*
	class ATOSE_ELF32_EHDR
	----------------------
	ELF file header
*/
class ATOSE_elf32_ehdr
{
public:
	/*
		Size of e_ident array
	*/
	static const uint32_t EI_NIDENT = 16;

	/*
		e_ident (file offsets of the magic numbers necessary to identify and ELF file)
	*/
	static const uint32_t EI_MAG0 = 0;
	static const uint32_t EI_MAG1 = 1;
	static const uint32_t EI_MAG2 = 2;
	static const uint32_t EI_MAG3 = 3;
	static const uint32_t EI_CLASS = 4;
	static const uint32_t EI_DATA = 5;
	static const uint32_t EI_VERSION = 6;
	static const uint32_t EI_PAD = 7;

	/*
		the EI_CLASS member of e_ident - we only care about 32-bit files
	*/
	static const uint32_t ELFCLASS32 = 1;

	/*
		the EI_DATA member of e_ident - we only care about little endian files
	*/
	static const uint32_t ELFDATA2LSB = 1;

	/*
		e_type values (we only care about EXE files
	*/
	static const uint32_t ET_EXEC = 2;

	/*
		e_machine values (we only care about ARM)
	*/
	static const uint32_t EM_ARM = 40;

	/*
		e_version - the version number if the ELF file format
	*/
	static const uint32_t EV_CURRENT = 1;

public:
	uint8_t e_ident[EI_NIDENT];	/* File identification. */
	uint16_t	e_type;
	uint16_t	e_machine;
	uint32_t	e_version;
	uint32_t	e_entry;
	uint32_t	e_phoff;
	uint32_t	e_shoff;
	uint32_t	e_flags;
	uint16_t	e_ehsize;
	uint16_t	e_phentsize;
	uint16_t	e_phnum;
	uint16_t	e_shentsize;
	uint16_t	e_shnum;
	uint16_t	e_shstrndx;
} ;

#endif
