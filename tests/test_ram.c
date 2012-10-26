/*
	TEST_RAM.C
	----------
	Load into low RAM and start touching pages to see if they exist
*/
#include <stdint.h>

#define UART_0_base_address  ((unsigned char *)0x101F1000)
volatile unsigned long *UART_0_data_register = (unsigned long *)(UART_0_base_address + 0x00);
volatile unsigned long *UART_0_recieve_status_register = (unsigned long *)(UART_0_base_address + 0x04);
volatile unsigned long *UART_0_error_clear_register = (unsigned long *)(UART_0_base_address + 0x04);
volatile unsigned long *UART_0_flag_register = (unsigned long *)(UART_0_base_address + 0x18);
volatile unsigned long *UART_0_irda_low_power_counter_register = (unsigned long *)(UART_0_base_address + 0x20);
volatile unsigned long *UART_0_integer_baud_rate_register = (unsigned long *)(UART_0_base_address + 0x24);
volatile unsigned long *UART_0_fractional_baud_rate_register = (unsigned long *)(UART_0_base_address + 0x28);
volatile unsigned long *UART_0_line_control_register = (unsigned long *)(UART_0_base_address + 0x2C);
volatile unsigned long *UART_0_control_register = (unsigned long *)(UART_0_base_address + 0x30);
volatile unsigned long *UART_0_interrupt_fifo_level_select_register = (unsigned long *)(UART_0_base_address + 0x34);
volatile unsigned long *UART_0_interrupt_mask_set_clear_register = (unsigned long *)(UART_0_base_address + 0x38);
volatile unsigned long *UART_0_raw_interrupt_status_register = (unsigned long *)(UART_0_base_address + 0x3C);
volatile unsigned long *UART_0_masked_interrupt_status_register = (unsigned long *)(UART_0_base_address + 0x40);
volatile unsigned long *UART_0_interrupt_clear_register = (unsigned long *)(UART_0_base_address + 0x44);
volatile unsigned long *UART_0_dma_control_register = (unsigned long *)(UART_0_base_address + 0x48);


/*
	DEBUG_ENABLE()
	--------------
*/
void debug_enable(void)
{
*UART_0_integer_baud_rate_register = 0x30;
*UART_0_fractional_baud_rate_register = 0x00;
*UART_0_line_control_register = (*UART_0_line_control_register & ~0xFF) | (1 << 5) | (1 << 6);
*UART_0_interrupt_fifo_level_select_register &= ~0x3F;
*UART_0_interrupt_mask_set_clear_register |= 1 << 4;
}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char ch)
{
*UART_0_data_register = ch;
}

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(int data)
{
int i = 0;
char c;

for (i = sizeof(data) * 2 - 1; i >= 0; i--)
	{
	c = data >> (i * 4);
	c &= 0xf;
	if (c > 9)
		debug_putc(c - 10 + 'A');
	else
		debug_putc(c + '0');
	}
}

/*
	DEBUG_PRINT_STRING()
	--------------------
*/
void debug_print_string(const char *string)
{
while (*string != 0)
	debug_putc(*string++);
}

/*
	C_ENTRY()
	---------
*/
void c_entry(void)
{
uint8_t *location;
uint8_t value;
uint8_t *start = (uint8_t *)(1024 * 1024);
uint8_t *end = (uint8_t *)(~0 - 1024 * 1024);

/*
	do the work
*/
for (location = start; location <= end; location += (1024 * 1024))
	{
	value = (uint32_t)location ^ ((uint32_t)location >> 8) ^ ((uint32_t)location >> 16) ^ ((uint32_t)location >> 24);
	*location = value;
	debug_print_hex((uint32_t)location);

	if (*location == value)
		debug_print_string(" Y\r\n");
	else
		debug_print_string(" N\r\n");
	}

for (;;);
}

