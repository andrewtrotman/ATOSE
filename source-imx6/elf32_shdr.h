/*
	ELF32_SHDR.H
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This header file includes the declarations of all the structures necessary
	to load and execute an EFL file
*/
#ifndef ELF32_SHDR_H_
#define ELF32_SHDR_H_

#include <stdint.h>

/*
	class ATOSE_ELF32_SHDR
	----------------------
	ELF section header
*/
class ATOSE_elf32_shdr
{
public:
	/*
		sh_flags (section flags)
	*/
	static const uint32_t SHF_WRITE = 0x01;
	static const uint32_t SHF_ALLOC = 0x02;
	static const uint32_t SHF_EXECINSTR = 0x04;

public:
	uint32_t	sh_name;
	uint32_t	sh_type;
	uint32_t	sh_flags;
	uint32_t	sh_addr;
	uint32_t	sh_offset;
	uint32_t	sh_size;
	uint32_t	sh_link;
	uint32_t	sh_info;
	uint32_t	sh_addralign;
	uint32_t	sh_entsize;
} ;


#endif
