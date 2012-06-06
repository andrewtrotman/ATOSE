/*
	REGISTERS.H
	-----------
	asm(code : output operand list : input operand list : clobber list);
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
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13;			// stack pointer
	uint32_t r14;			// link register
	uint32_t r15;			// program counter
//	uint32_t cpsr;			// flags

public:
	inline void get(void)
		{
		asm volatile ("mov %0, r0" : "=r"(r0));
		asm volatile ("mov %0, r1" : "=r"(r1));
		asm volatile ("mov %0, r2" : "=r"(r2));
		asm volatile ("mov %0, r3" : "=r"(r3));
		asm volatile ("mov %0, r4" : "=r"(r4));
		asm volatile ("mov %0, r5" : "=r"(r5));
		asm volatile ("mov %0, r6" : "=r"(r6));
		asm volatile ("mov %0, r7" : "=r"(r7));
		asm volatile ("mov %0, r8" : "=r"(r8));
		asm volatile ("mov %0, r9" : "=r"(r9));
		asm volatile ("mov %0, r10" : "=r"(r10));
		asm volatile ("mov %0, r11" : "=r"(r11));
		asm volatile ("mov %0, r12" : "=r"(r12));
		asm volatile ("mov %0, r13" : "=r"(r13));
		asm volatile ("mov %0, r14" : "=r"(r14));
		asm volatile ("mov %0, r15" : "=r"(r15));
		}

	inline void set(void)
		{
		asm volatile ("mov r0, %0" :: "r"(r0));
		asm volatile ("mov r1, %0" :: "r"(r1));
		asm volatile ("mov r2, %0" :: "r"(r2));
		asm volatile ("mov r3, %0" :: "r"(r3));
		asm volatile ("mov r4, %0" :: "r"(r4));
		asm volatile ("mov r5, %0" :: "r"(r5));
		asm volatile ("mov r6, %0" :: "r"(r6));
		asm volatile ("mov r7, %0" :: "r"(r7));
		asm volatile ("mov r8, %0" :: "r"(r8));
		asm volatile ("mov r9, %0" :: "r"(r9));
		asm volatile ("mov r10, %0" :: "r"(r10));
		asm volatile ("mov r11, %0" :: "r"(r11));
		asm volatile ("mov r12, %0" :: "r"(r12));
		asm volatile ("mov r13, %0" :: "r"(r13));
		asm volatile ("mov r14, %0" :: "r"(r14));
		asm volatile ("mov r15, %0" :: "r"(r15));
		}
} ;

#endif /* REGISTERS_H_ */
