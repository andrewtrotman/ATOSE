/*
	TEST_SWI.C
	----------
*/
#include <stdint.h>
#include <stdio.h>

uint32_t number_of_swi = 0;

/*
	__CS3_ISR_SWI()
	---------------
*/
void __attribute__ ((interrupt ("SWI"))) __cs3_isr_swi(void)
{
puts(".");
number_of_swi++;
}


/*
	MAIN()
	------
*/
int main(void)
{
puts("Enter");

asm volatile ("swi 121");

puts("Exit");
}

