/*
	ELF32_PHDR.H
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This header file includes the declarations of all the structures necessary
	to load and execute an EFL file
*/
#ifndef ELF32_PHDR_H_
#define ELF32_PHDR_H_

#include <stdint.h>

/*
	class ATOSE_ELF32_PHDR
	----------------------
	ELF program header
*/
class ATOSE_elf32_phdr
{
public:
	/*
		p_type values (we only care about what EXE relevant ones)
	*/
	static const uint32_t PT_ARM_UNWIND = 0x70000001;
	static const uint32_t PT_LOAD = 1;

	/*
		p_flags (section flags)
	*/
	static const uint32_t PF_X = 0x01;	// execute permitted
	static const uint32_t PF_W = 0x02;	// write permitted
	static const uint32_t PF_R = 0x04;	// read permitted

public:
	uint32_t	p_type;
	uint32_t	p_offset;
	uint32_t	p_vaddr;
	uint32_t	p_paddr;
	uint32_t	p_filesz;
	uint32_t	p_memsz;
	uint32_t	p_flags;
	uint32_t	p_align;
} ;

#endif
