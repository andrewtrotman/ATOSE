/*
	TIMER.C
	-------
	Verify the system timer interrupt mechanism (this is written for the ARM versatile ab board)
*/
#include <stdio.h>

volatile long *ticks = (long *)((unsigned char *)0x101E2000 + 0x04);
volatile long happened = 0;
volatile long global_sp = 0;


/*
	Programmable Interrupt controller
*/

unsigned char *PIC_base_address = (unsigned char *)0x10140000;

volatile unsigned long *PIC_IRQ_status_register = (unsigned long *)(PIC_base_address + 0x00);
volatile unsigned long *PIC_FIRQ_status_register = (unsigned long *)(PIC_base_address + 0x04);
volatile unsigned long *PIC_RAW_status_register = (unsigned long *)(PIC_base_address + 0x08);
volatile unsigned long *PIC_interrupt_select_register = (unsigned long *)(PIC_base_address + 0x0C);
volatile unsigned long *PIC_interrupt_enable_register = (unsigned long *)(PIC_base_address + 0x10);
volatile unsigned long *PIC_interrupt_enable_clear_register = (unsigned long *)(PIC_base_address + 0x14);
volatile unsigned long *PIC_software_interrupt_register = (unsigned long *)(PIC_base_address + 0x18);
volatile unsigned long *PIC_software_interrupt_clear_register = (unsigned long *)(PIC_base_address + 0x1C);
volatile unsigned long *PIC_protection_enable_register = (unsigned long *)(PIC_base_address + 0x20);
volatile unsigned long *PIC_vector_address_register = (unsigned long *)(PIC_base_address + 0x30);
volatile unsigned long *PIC_default_vector_address_register = (unsigned long *)(PIC_base_address + 0x34);
volatile unsigned long *PIC_vector_address_registers = (unsigned long *)(PIC_base_address + 0x100);
volatile unsigned long *PIC_vector_control_registers = (unsigned long *)(PIC_base_address + 0x200);
volatile unsigned long *PIC_peripheral_id_register = (unsigned long *)(PIC_base_address + 0xFE0);

/*
	Timer
*/
unsigned char *timer_base_address = (unsigned char *)0x101E2000;

volatile unsigned long *TimerXLoad = (unsigned long *)(timer_base_address + 0x00);
volatile unsigned long *TimerXValue = (unsigned long *)(timer_base_address + 0x04);
volatile unsigned long *TimerXControl = (unsigned long *)(timer_base_address + 0x08);
volatile unsigned long *TimerXIntClr = (unsigned long *)(timer_base_address + 0x0C);
volatile unsigned long *TimerXRIS = (unsigned long *)(timer_base_address + 0x10);
volatile unsigned long *TimerXMIS = (unsigned long *)(timer_base_address + 0x14);
volatile unsigned long *TimerXBGLoad = (unsigned long *)(timer_base_address + 0x18);


/*
	GET_CPSR()
	----------
*/
inline int get_cpsr(void)
{ 
int answer;

asm volatile ("mrs %0,CPSR":"=r" (answer));

return answer;
}

/*
	SET_CPSR()
	----------
*/
inline void set_cpsr(int save_cpsr)
{
asm volatile ("msr CPSR_cxsf, %0"::"r"(save_cpsr));
}

/*
	ENABLE_IRQ()
	------------
*/
void enable_IRQ(void)
{
set_cpsr(get_cpsr() & ~0x80);
}

/*
	DISABLE_IRQ()
	-------------
*/
void disable_IRQ(void)
{
set_cpsr(get_cpsr() | 0x80);
}


/*
	GET_SP()
	--------
*/
inline unsigned int get_sp(void)
{
unsigned int answer;

asm volatile ("mov %0,sp":"=r" (answer));

return answer;
}

/*
	__CS3_ISR_IRQ()
	---------------
*/
extern "C" {
		void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq(void)
		{
		volatile unsigned long isr;

		/*
			Read from the IRQ vector address register to signal to the interrupt controller
			that we are servicing the interrupt

		*/
		isr = *PIC_vector_address_register;

		happened++;

		asm volatile ("mov %0,sp":"=r" (global_sp));

		/*
			Tell the timer that we've serviced the interrupt
		*/

		*TimerXIntClr = 0;

		/*
			Tell the interrupt controller that we're done servicing the interrupt
		*/
		*PIC_vector_address_register = isr;
		}
}

/*
	__CS3_ISR_FIQ()
	---------------
*/
void __attribute__ ((interrupt ("FIQ"))) cs3_isr_fiq(void)
{
happened++;
}

/*
	START_TIMER()
	-------------
*/
void start_timer(void)
{
puts("enable IRQ");
puts("");

{
extern void *__cs3_interrupt_vector_arm[];
for (int x = 0; x < 16; x++)
	printf("%d %p\n", x, __cs3_interrupt_vector_arm[x]);

printf("IRQ handler:%x\n", (unsigned)__cs3_isr_irq);
__cs3_interrupt_vector_arm[14] = (void *)__cs3_isr_irq;
for (int x = 0; x < 16; x++)
	printf("%d %p\n", x, __cs3_interrupt_vector_arm[x]);
}
enable_IRQ();

puts("Done");
puts("");



/*
	Enable the interrupt controller
*/
unsigned long got;

got = *PIC_IRQ_status_register;
printf("IRQ_status:%08x\n", got);

got = *PIC_interrupt_select_register;
printf("interrupt_select_register:%08x\n", got);

got = *PIC_interrupt_enable_register;
printf("interrupt_enable_register:%08x\n", got);

got = *PIC_default_vector_address_register;
printf("default_vector_address_register:%08x\n", got);

got = *PIC_vector_address_register;
printf("vector_address_register:%08x\n", got);

puts("Enable timer 0/1");

*PIC_interrupt_enable_register = 0x10;

got = *PIC_IRQ_status_register;
printf("IRQ_status:%08x\n", got);

got = *PIC_interrupt_select_register;
printf("interrupt_select_register:%08x\n", got);

got = *PIC_interrupt_enable_register;
printf("interrupt_enable_register:%08x\n", got);

got = *PIC_default_vector_address_register;
printf("default_vector_address_register:%08x\n", got);

got = *PIC_vector_address_register;
printf("vector_address_register:%08x\n", got);

/*
	Timer
*/
puts("");
puts("Enable timer\n");

unsigned long was;

/*
	Enable the timer
*/
*TimerXLoad = (unsigned long)0x8000;
was = *TimerXControl;
was = was & 0xFFFFFF10 | 0xE0;
*TimerXControl = was;

puts("Done");
puts("");

puts("enable IRQ");
puts("");

}

/*
	MAIN()
	------
*/
int main(void)
{
int x;
unsigned int my_sp;

my_sp = get_sp();

printf("Sizeof(int): %d\n", sizeof(int));
printf("Sizeof(long): %d\n", sizeof(long));
printf("Sizeof(long long): %d\n", sizeof(long long));

start_timer();

for (x = 0; x < 100; x++)
	printf("Ticks:%X interrupts:%X my_stack:%X global_stack:%X\n", *ticks, happened, my_sp, global_sp);
}


