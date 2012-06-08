/*
	REGISTERS.H
	-----------
	asm(code : output operand list : input operand list : clobber list);
*/
#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <stdint.h>
#include <string.h>

/*
	struct ATOSE_REGISTERS
	----------------------
*/
	typedef struct 
	{
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
	uint32_t r10;			// GCC=sl                   (firq specific)
	uint32_t r11;			// GCC=fp                   (firq specific)
	uint32_t r12;			// GCC=ip                   (firq specific)
	uint32_t r13;			// stack pointer			(mode specific)
	uint32_t r14;			// link register			(mode specific)
//	uint32_t r15;			// program counter
//	uint32_t cpsr;			// flags
	} ATOSE_registers;

#endif /* REGISTERS_H_ */
