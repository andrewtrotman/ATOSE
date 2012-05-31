/*
	IO_SERIAL.C
	-----------
*/
#include "ascii_str.h"
#include "io_serial.h"

unsigned char *UART_0_base_address = (unsigned char *)0x101F1000;
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
	ATOSE_IO_SERIAL::ENABLE()
	-------------------------
*/
void ATOSE_IO_serial::enable(void)
{
/*
	select 9600 baud rate
*/
*UART_0_integer_baud_rate_register = 0x30;
*UART_0_fractional_baud_rate_register = 0x00;

/*
	n,8,1
*/
*UART_0_line_control_register = (*UART_0_line_control_register & ~0xFF) | (1 << 5) | (1 << 6);

/*
	Enable recieve interrupts (interrupt on 1 character in FIFO)
*/
*UART_0_interrupt_fifo_level_select_register &= ~0x3F;
*UART_0_interrupt_mask_set_clear_register |= 1 << 4;			// interrupt on recieve (RXIM)
}

/*
	ATOSE_IO_SERIAL::DISABLE()
	--------------------------
*/
void ATOSE_IO_serial::disable(void)
{
/*
	Disable UART interrupts
*/
*UART_0_interrupt_mask_set_clear_register &= ~0x7FF;
}

/*
	ATOSE_IO_SERIAL::ACKNOWLEDGE()
	------------------------------
*/
void ATOSE_IO_serial::acknowledge(void)
{
unsigned char got;
/*
	Clear the interrupt bits
*/
*UART_0_interrupt_clear_register = 0x7FF;

/*
	shove the key press into the I/O buffers
*/
got = *UART_0_data_register;
push(&got);
}

/*
	ATOSE_IO_SERIAL::PUSH()
	-----------------------
*/
void ATOSE_IO_serial::push(unsigned char *byte)
{
buffer.write(*byte);
}

/*
	ATOSE_IO_SERIAL::WRITE()
	------------------------
*/
int ATOSE_IO_serial::write(const char *buffer, int bytes)
{
int current;

for (current = 0; current < bytes; current++)
	*UART_0_data_register = buffer[current];

return bytes;
}

/*
	ATOSE_IO_SERILA::READ()
	-----------------------
*/
int ATOSE_IO_serial::read(char *into, int bytes)
{
int which;

for (which = 0; which < bytes; which++)
	{
	if (buffer.is_empty())
		return which;
	*into++ = buffer.read();
	}

return bytes;
}

