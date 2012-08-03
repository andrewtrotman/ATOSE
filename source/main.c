/*
	MAIN.C
	------
	Entry point to ATOSE
*/
#include <stdint.h>
#include "atose.h"
#include "api_atose.h"

#ifdef IMX233
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#endif


extern "C" { int main2(void); }

extern "C"
{
	/*
		C_ENTRY()
		---------
	*/
	void c_entry() 
	{
	#ifdef FourARM
		/*
			Magic to get around the brownout problem in the FourARM
		*/
		HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;
	#endif
	/*
		to start with we'll make sure the interrupt vectors are correct
	*/
	extern uint32_t ATOSE_vectors_start;
	extern uint32_t ATOSE_vectors_end;
	uint32_t *vectors_src = &ATOSE_vectors_start;
	uint32_t *vectors_dst = (uint32_t *)0;

	while(vectors_src < &ATOSE_vectors_end)
		*vectors_dst++ = *vectors_src++;
	/*
		then we'll call all the constructors
	*/

	extern void (*start_ctors)();
	extern void (*end_ctors)();
	void (**constructor)() = &start_ctors;

	while (constructor < &end_ctors)
		{
		(*constructor)();
		constructor++;
		}

	/*
		Call main
	*/
	extern int main(void);
	main();
	}

	/*
		Stubs necessary to get the compiler to work
	*/
	/*
		ABORT()
		-------
	*/
	void abort(void)
	{
	}

	/*
		_SBRK_R ()
		----------
	*/
	void *_sbrk_r(int inc)
	{
	return 0;
	}
}

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE os;

/*
	Switch to user mode 
*/
asm volatile
	(
	"mrs r0, CPSR;"
	"bic r0, r0, #0x01F;"
	"orr r0, r0, #0x10;"
	"msr cpsr_cxsf, r0;"
	:
	:
	: "r0"
	);

//asm volatile ( "mov %0, sp;" : "=r"(tos2));


return main2();
}

int main2(void)
{

ATOSE_API_ATOSE api;
api.io.hex();

//api.io << "system:" << (long)tos1 << " user:" << (long)tos2 << ATOSE_IO::eoln;
//api.io << "kbd_id1:" << api.keyboard.id_object << ATOSE_IO::eoln;
//api.io << "Mouse_id1:" << api.mouse.id_object << ATOSE_IO::eoln;
//uint32_t ans;

//api.io << "Mouse_id2:" << api.mouse.id_object << ATOSE_IO::eoln;
//api.io.hex();
//api.io << "Mouse_id2a:" << api.mouse.id_object << ATOSE_IO::eoln;
//api.io.decimal();
//api.io << "Mouse_id2b:" << api.mouse.id_object << ATOSE_IO::eoln;
//api.io.hex();
//api.io << "Mouse_id2c:" << api.mouse.id_object << ATOSE_IO::eoln;


extern uint32_t ATOSE_top_of_memory;

//api.io << "Mouse_id3:" << api.mouse.id_object << ATOSE_IO::eoln;
//api.keyboard.write_byte(0xFF);
//api.io << "Mouse_id4:" << api.mouse.id_object << ATOSE_IO::eoln;
//api.mouse.write_byte(0xFF);
//api.mouse.write_byte(0xF4);
//api.io << "Mouse_id5:" << api.mouse.id_object << ATOSE_IO::eoln;


//HW_UARTDBGCR_SET(BM_UARTDBGCR_RXE);		// turn on rec

for (;;)
	{
	uint8_t got;

//	if (api.keyboard.read_byte(&got))
//		api.io << "KBM: " << (long)got << ATOSE_IO::eoln;
//	if (api.mouse.read_byte(&got))
//		api.io << "MOU: " << (long)got << ATOSE_IO::eoln;
	if (api.io.read_byte(&got))
		{
		api.io << "COM: " << (long)got << " '" << got << "'" << ATOSE_IO::eoln;
//		ans = api.mouse.read_byte(&got);
//		api.io << "API got: " << (long)got << " ans:" << (long)ans << ATOSE_IO::eoln;
		}

/*
	uint32_t uart_status = HW_UARTDBGRIS_RD();
	uint32_t uart_rsr = HW_UARTDBGRSR_ECR_RD();
	uint32_t uart_fr = HW_UARTDBGFR_RD();
	uint32_t uart_lcr = HW_UARTDBGLCR_H_RD();
	uint32_t uart_cr = HW_UARTDBGCR_RD();
*/

//	api.io << "DGB: status=" << (long)uart_status << "; rsr=" << uart_rsr << "; rf=" << uart_fr << "; lcr="<< uart_lcr << "; cr=" << uart_cr << ATOSE_IO::eoln;
//	HW_UARTDBGICR_CLR(0x7FF);
	}

//api.io << "Mouse_id6:" << api.mouse.id_object << ATOSE_IO::eoln;

return 0;
}
