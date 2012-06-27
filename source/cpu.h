/*
	CPU.H
	-----
*/
#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

/*
	class ATOSE_CPU
	---------------
*/
class ATOSE_cpu
{
private:
	int get_cpsr(void);
	void set_cpsr(uint32_t save_cpsr);

public:
	ATOSE_cpu() {}

	void init(void);

	void enable_IRQ(void);
	void disable_IRQ(void);

	void move_interrupt_vector_table_to_zero(void);

	void enter_user_mode(void);
} ;

#endif /* CPU_H_ */




