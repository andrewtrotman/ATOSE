/*
	CPU_ARM.H
	---------
*/
#ifndef CPU_ARM_H_
#define CPU_ARM_H_

#include <stdint.h>

/*
	class ATOSE_CPU_ARM
	-------------------
*/
class ATOSE_cpu_arm
{
private:
	static const uint32_t irq_stack_size = 1024;		// size of the IRQ stack (in bytes). This will be rounded down to the nearest whole word

private:
	uint32_t get_cpsr(void);
	void set_cpsr(uint32_t new_cpsr);
	void irq_handler(void);

public:
	cpu_arm();
	virtual void set_irq_handler(void *address) = 0;
	void enable_irq(void);
} ;

#endif
