/*
	INTERRUPTS.C
	------------
	this file contains all the interrupt handlers (also known as interrupt service rountines)
	we rely on the interrupt vector table already existing and only provide the routines here.
*/

#define AngelSWI_ARM	0x123456
#ifdef __thumb__
	#define AngelSWI    0xAB
#else
	#define AngelSWI    AngelSWI_ARM
#endif

#define AngelSWI_Reason_WriteC          0x03
#define AngelSWI_Reason_Write0          0x04

/*
	DO_ANGEL_SWI()
	--------------
*/
inline int do_Angel_SWI(int reason, void *arg)
{
int value;

asm volatile ("mov r0, %1; mov r1, %2; swi %a3; mov %0, r0"
    : "=r" (value) /* Outputs */
    : "r" (reason), "r" (arg), "i" (AngelSWI) /* Inputs */
    : "r0", "r1", "lr"
             /* Clobbers r0 and r1, and lr if in supervisor mode */);

return value;
}

/*
	ATOSE_DEFAULT_ISR()
	-------------------
*/
void ATOSE_default_isr(void)
{
unsigned char debug_message;

do_Angel_SWI(AngelSWI_Reason_Write0, (void *)"INTERRUPT");
}

/*
	__CS3_RESET()
	-------------
*/
void __attribute__ ((interrupt)) __cs3_reset(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_UNDEF()
	-----------------
*/
void __attribute__ ((interrupt)) __cs3_isr_undef(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_SWI()
	---------------
*/
void __attribute__ ((interrupt)) __cs3_isr_swi(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_PABORT()
	------------------
*/
void __attribute__ ((interrupt)) __cs3_isr_pabort(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_DABORT()
	------------------
*/
void __attribute__ ((interrupt)) __cs3_isr_dabort(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_RESERVED()
	--------------------
*/
void __attribute__ ((interrupt)) __cs3_isr_reserved(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_IRQ()
	---------------
*/
void __attribute__ ((interrupt)) __cs3_isr_irq(void)
{
ATOSE_default_isr();
}

/*
	__CS3_ISR_FIQ()
	---------------
*/
void __attribute__ ((interrupt)) __cs3_isr_fiq(void)
{
ATOSE_default_isr();
}

