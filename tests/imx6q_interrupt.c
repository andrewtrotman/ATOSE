/*
	IMX6Q_INTERRUPT.C
	-----------------
*/
#include <stdint.h>

#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsgpt.h"

#define BAUD_RATE 115200
#define DEFAULT_UART 2		/* can be either 2 (SABRE Lite "console") or 1 (the other UART) */
#define PLL3_FREQUENCY 80000000

/*
	SERIAL_INIT()
	-------------
*/
void serial_init(void)
{
/*
   Enable Clocks
*/
*((uint32_t *)0x020C407C) = 0x0F0000C3;	// CCM Clock Gating Register 5 (CCM_CCGR5) (inc UART clock)

/*
   Enable Pads
*/
#if (DEFAULT_UART == 1)
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
#elif (DEFAULT_UART == 2)
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
#else
	#error "Only UART 1 and 2 are supported"
#endif

/*
	Now on to the UART 8 bits, 1 stop bit, no parity, software flow control
*/
HW_UART_UCR1(DEFAULT_UART).U = 0;
HW_UART_UCR2(DEFAULT_UART).U = 0;
while (HW_UART_UCR2(DEFAULT_UART).B.SRST == 0)
	;	// nothing

HW_UART_UCR1(DEFAULT_UART).B.UARTEN = 1;
HW_UART_UCR3(DEFAULT_UART).B.RXDMUXSEL = 1;
HW_UART_UCR2_WR(DEFAULT_UART, BM_UART_UCR2_WS | BM_UART_UCR2_IRTS | BM_UART_UCR2_RXEN | BM_UART_UCR2_TXEN | BM_UART_UCR2_SRST);
HW_UART_UFCR(DEFAULT_UART).B.RFDIV = 0x04;		/* divide input clock by 2 */
HW_UART_UBIR(DEFAULT_UART).U = 0x0F;
HW_UART_UBMR(DEFAULT_UART).U = (PLL3_FREQUENCY / (HW_CCM_CSCDR1.B.UART_CLK_PODF + 1)) / (2 * BAUD_RATE);		// UBMR should be 0x015B once set
}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char value)
{
HW_UART_UTXD(DEFAULT_UART).U = value;
while (HW_UART_UTS(DEFAULT_UART).B.TXEMPTY == 0)
	; // do nothing
}

/*
	DEBUG_PUTS()
	------------
*/
void debug_puts(char *string)
{
while (*string != '\0')
	debug_putc(*string++);
}

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(int data)
{
int i = 0;
char c;

for (i = sizeof(int)*2-1; i >= 0; i--)
	{
	c = data>>(i*4);
	c &= 0xf;
	if (c > 9)
		debug_putc(c-10+'A');
	else
		debug_putc(c+'0');
	}
}

/*
   TIMER_INIT()
   ------------
*/
void timer_init(void)
{
HW_GPT_CR.B.EN = 0;
HW_GPT_IR.U = 0;
HW_GPT_CR.B.OM1 = HW_GPT_CR.B.OM2 = HW_GPT_CR.B.OM3 = 0;
HW_GPT_CR.B.IM1 = HW_GPT_CR.B.IM2 = 0;
HW_GPT_CR.B.CLKSRC = 0x07;      // use the 24MHz crystal
HW_GPT_CR.B.SWR = 0;
while (HW_GPT_CR.B.SWR != 0)
   ;/* nothing */
HW_GPT_SR.U = 0;
HW_GPT_CR.B.ENMOD = 1;
HW_GPT_CR.B.EN = 1;
}

/*
	ARM_GENERIC_INTERRUPT_CONTROLLER_DISTRIBUTOR_REGISTER_MAP
	---------------------------------------------------------
	See page 4-2 to 4-4 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
	There should be one of these per interrupt controller (one per die)
*/
typedef struct
{
uint32_t distributor_control_register;							// 0x0000
uint32_t interrupt_controller_type_register;					// 0x0004
uint32_t distributor_implementer_identification_register;		// 0x0008
uint32_t reserved1[29];										// 0x000C
uint32_t interrupt_security_registers[32];						// 0x0080
uint32_t interrupt_set_enable_registers[32];					// 0x0100
uint32_t interrupt_clear_enable_registers[32];					// 0x0180
uint32_t interrupt_set_pending_registers[32];					// 0x0200
uint32_t interrupt_clear_pending_registers[32];				// 0x0280
uint32_t active_bit_registers[32];								// 0x0300
uint32_t reserved2[32];										// 0x0380
uint32_t interrupt_priority_registers[255];					// 0x0400
uint32_t reserved3;											// 0x07FC
uint32_t interrupt_processor_targets_registers[8];				// 0x0800
uint32_t reserved4[247];										// 0x0820
uint32_t reserved5;											// 0x0BFC
uint32_t interrupt_configuration_registers[64];				// 0x0C00
uint32_t implementation_defined_registers[64];					// 0x0D00
uint32_t reserved6[64];										// 0x0E00
uint32_t software_generated_interrupt_register;				// 0x0F00
uint32_t reserved7[51];										// 0x0F04
uint32_t peripheral_id4;										// 0x0FD0	the next 12 32-bit words are implementation defined identification_registers
uint32_t peripheral_id5;										// 0x0FD4
uint32_t peripheral_id6;										// 0x0FD8
uint32_t peripheral_id7;										// 0x0FDC
uint32_t peripheral_id0;										// 0x0FE0
uint32_t peripheral_id1;										// 0x0FE4
uint32_t peripheral_id2;										// 0x0FE8
uint32_t peripheral_id3;										// 0x0FEC
uint32_t component_id0;										// 0x0FF0
uint32_t component_id1;										// 0x0FF4
uint32_t component_id2;										// 0x0FF8
uint32_t component_id3;										// 0x0FFC
} ARM_generic_interrupt_controller_distributor_register_map;

/*
	ARM_GENERIC_INTERRUPT_CONTROLLER_CPU_REGISTER_MAP
	-------------------------------------------------
	See page 4-4 to 4-5 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
	There should be one of these per core
*/
typedef struct
{
uint32_t cpu_interface_control_register;						// 0x00
uint32_t interrupt_priority_mask_register;						// 0x04
uint32_t binary_point_register;								// 0x08
uint32_t interrupt_acknowledge_register;						// 0x0C
uint32_t end_of_interrupt_register;							// 0x10
uint32_t running_priority_register;							// 0x14
uint32_t highest_pending_interrupt_register;					// 0x18
uint32_t aliased_binary_point_register;						// 0x1C
uint32_t reserved1[8];											// 0x20
uint32_t implementation_defined_registers[36];					// 0x40
uint32_t reserved2[11];										// 0xD0
uint32_t cpu_interface_dentification_register;					// 0xFC
} ARM_generic_interrupt_controller_cpu_register_map;

/*
   MAIN()
   ------
*/
int main(void)
{
ARM_generic_interrupt_controller_cpu_register_map *cpu_registers;
ARM_generic_interrupt_controller_distributor_register_map *distributor_registers;
uint32_t base;
long x;
uint32_t count, rollover;

serial_init();
debug_puts("\r\nStart:" __TIME__ "\r\n");
timer_init();
debug_puts("Timer initilised\r\n");

HW_GPT_OCR1.U = 0x100;     // fun until the GPT gets to 0x100
HW_GPT_CR.B.FRR = 0;    	// restart mode (periodic interrupt mode)

/*
	Get the address of the CPU's configuration registers, but in the address space of the SOC.
	This instruction only works on the Cortex-A9 MPCore, it does not work on a unicore Cortex A9.
	It has been tested on the i.MX6Q
*/
asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);

cpu_registers = (ARM_generic_interrupt_controller_cpu_register_map *)(base + 0x100);
distributor_registers = (ARM_generic_interrupt_controller_distributor_register_map *)(base + 0x1000);

/*
	On the Freescale i.MX6Q this should produce:00A00000
*/
debug_puts("ARM Registers Address:");
debug_print_hex(base);
debug_puts("\r\n");

/*
	On the Freescale i.MX6Q this should produce:3901243B
*/
debug_puts("cpu_interface_dentification_register:");
debug_print_hex(cpu_registers->cpu_interface_dentification_register);
debug_puts("\r\n");

/*
	On the Freescale i.MX6Q this should produce: 0000000D 000000F0 00000005 000000B1
*/
debug_puts("Component ID:");
debug_print_hex(distributor_registers->component_id0);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id1);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id2);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id3);
debug_puts("\r\n");

for (x = 0; x < 10; x++)
   {
   count = HW_GPT_CNT_RD();
   rollover = HW_GPT_SR_RD();

   debug_puts("GPT_CNT:");
   debug_print_hex(count);
   debug_puts(" ");

   debug_puts("GPT_SR:");
   debug_print_hex(rollover);
   debug_puts(" ");
   if (rollover != 0)
      HW_GPT_SR.B.OF1 = 1;       // clear the roll_over bit

   debug_puts("\r\n");
   }

return 0;
}
