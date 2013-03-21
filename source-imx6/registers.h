/*
	REGISTERS.H
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <stdint.h>

/*
	class ATOSE_REGISTERS
	---------------------
*/
class ATOSE_registers
{
public:
	uint32_t cpsr;				// flags
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;									//  (firq specific)
	uint32_t r9;									//  (firq specific)
	uint32_t r10;				// GCC=sl                   (firq specific)
	uint32_t r11;				// GCC=fp                   (firq specific)
	uint32_t r12;				// GCC=ip                   (firq specific)
	uint32_t r13;				// stack pointer			(mode specific)
	uint32_t r14;				// link register			(mode specific)
	uint32_t r14_current;		// current mode's R14 register
	//	uint32_t r15;			// program counter - we don't need to save this because its transferred into the link register
} ;

#endif /* REGISTERS_H_ */
