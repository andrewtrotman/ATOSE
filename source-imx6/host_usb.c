/*
	HOST_USB.C
	----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

/*
	We need to tell the i.MX6 SDK that we're using an i.MX6Q
*/
#define CHIP_MX6DQ 1

#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccmanalog.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbcore.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbphy.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbanalog.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsgpio.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsepit.h"

#include "atose.h"
#include "atose_api.h"
#include "host_usb.h"
#include "ascii_str.h"
#include "usb_ehci_queue_head.h"
#include "usb_standard_hub_descriptor.h"
#include "usb_standard_device_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_standard_configuration_descriptor.h"
#include "usb_ehci_queue_head_endpoint_capabilities.h"
#include "usb_ehci_queue_element_transfer_descriptor.h"
#include "usb_ehci_queue_head_horizontal_link_pointer.h"
#include "usb_ehci_queue_head_endpoint_characteristics.h"

#include "../systems/iMX6_Platform_SDK/sdk/drivers/gpio/gpio.h"
#define INVALID_PARAMETER (-1)
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/iomux_register.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/iomux_define.h"

/*
	=====================================================
	=====================================================
	=====================================================
*/
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"

#define DEFAULT_UART 2
#define BAUD_RATE 115200
#define PLL3_FREQUENCY 80000000

void dump_usbsts(void);
void do_some_magic(void);


/*
	DEBUG_INIT()
	------------
*/
void debug_init(void)
{
*((uint32_t *)0x020C407C) = 0x0F0000C3;	// CCM Clock Gating Register 5 (CCM_CCGR5) (inc UART clock)

HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));

HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));

HW_UART_UCR1(DEFAULT_UART).U = 0;						// disable the UART
HW_UART_UCR2(DEFAULT_UART).U = 0;						// software reset (SRST)
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
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(int data)
{
int i = 0;
char c;

for (i = sizeof(int) * 2 - 1; i >= 0; i--)
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
	DEBUG_PRINT_HEX_BYTE()
	----------------------
*/
void debug_print_hex_byte(uint8_t data)
{
const char *string = "0123456789ABCDEF";

debug_putc(string[(data >> 4) & 0x0F]);
debug_putc(string[data & 0x0F]);
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
	DEBUG_PRINT_THIS()
	------------------
*/
void debug_print_this(const char *start, uint32_t hex, const char *end = "")
{
debug_print_string(start);
debug_print_hex(hex);
debug_print_string(end);
debug_print_string("\r\n");
}

/*
	DEBUG_PRINT_CF_THIS()
	---------------------
*/
void debug_print_cf_this(const char *start, uint32_t hex1, uint32_t hex2, const char *end = "")
{
debug_print_string(start);
debug_print_hex(hex1);
debug_print_string(hex1 == hex2 ? "  = " : " != ");
debug_print_hex(hex2);
debug_print_string(end);
debug_print_string("\r\n");
}

long isprint(int c)
{
if (c > ' ' && c <= 127)
	return 1;

return 0;
}

/*
	DEBUG_DUMP_BUFFER()
	-------------------
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes)
{
uint64_t remaining, width, column;

remaining = bytes;
while (remaining > 0)
	{
	debug_print_hex(address);
	debug_print_string(" ");

	width = remaining > 0x10 ? 0x10 : remaining;

	for (column = 0; column < width; column++)
		{
		debug_print_hex_byte(buffer[column]);
		debug_print_string(" ");
		}

	for (; column < 0x10; column++)
		debug_print_string("   ");

	debug_print_string(" ");
	for (column = 0; column < width; column++)
		debug_putc(isprint(buffer[column]) ? buffer[column] : '.');

	debug_print_string("\r\n");
	buffer += width;
	address += width;
	remaining -= width;
	}
}


/*
	=====================================================
	=====================================================
	=====================================================
*/
#ifdef SABRE_LITE
	/*
		SABRE_LITE_HUB_RESET()
		----------------------
		The SABRE Lite board has a 3-port USB hub connected to USB Port 1.  If we want to connect to a device to the
		board its going to go through this hub and so we need to reset the hub.  The USB Hub is connected to pin 12 of
		GPIO port 7.
	*/
	void SABRE_Lite_hub_reset(void)
	{
	/*
		Enable the pads (this code was generated by the IOMUX tool)
	*/
	HW_IOMUXC_SW_MUX_CTL_PAD_GPIO17_WR(BF_IOMUXC_SW_MUX_CTL_PAD_GPIO17_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_GPIO17_MUX_MODE_V(ALT5));
	HW_IOMUXC_SW_PAD_CTL_PAD_GPIO17_WR(BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_SRE_V(SLOW));

	/*
		Set the direction to output.  The Hub is connected to pin 12 of GPIO port 7
	*/
	HW_GPIO_GDIR_SET(7, 1 << 12);
	ATOSE_atose::get_ATOSE()->cpu.delay_us(20);							// I think we have to wait 2 "wait states", but I'm not sure how long that is. 20us seems to work

	/*
		Hold the line high (in the 1-state)
	*/
	HW_GPIO_DR_SET(7, 1 << 12);
	}
#endif
/*
	=====================================================
	=====================================================
	=====================================================
*/


/*
	ATOSE_HOST_USB::ATOSE_HOST_USB()
	--------------------------------
*/
ATOSE_host_usb::ATOSE_host_usb() : ATOSE_device_driver()
{
/*
	USB Core 1 is different from Core 0 because its not an OTG port. It differs from
	Core 2 and 3 because it is UTMI - that is, it has a Phy.  It supports "High Speed / Full Speed / Low Speed operation"
	according to page 5188 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	Somewhat confusingly, "USBPHY2 is the PHY interface for USB Host1 controller" (page 5454 of "i.MX 6Dual/6Quad Applications
	Processor Reference Manual Rev. 0, 11/2012")  I guess they count ports from 0 but Phy from 1.

	Page 5461 states:
		"The register settings in this section are recommended for passing USB certification.
		The following settings lower the J/K levels to certifiable limits:
			HW_USBPHY_TX_TXCAL45DP = 0x0
			HW_USBPHY_TX_TXCAL45DN = 0x0
			HW_USBPHY_TX_D_CAL = 0x7"
*/

/*
	Enable the pads (this code was generated by the IOMUX tool)
*/
HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_MUX_MODE_V(ALT6));
HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_SRE_V(SLOW));
HW_IOMUXC_USB_H1_OC_SELECT_INPUT_WR(BF_IOMUXC_USB_H1_OC_SELECT_INPUT_DAISY_V(EIM_DATA30_ALT6));

/*
	Send the clock the the Phy
*/
HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_POWER);
HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_EN_USB_CLKS);
while(!(HW_CCM_ANALOG_PLL_USB2_RD() & BM_CCM_ANALOG_PLL_USB2_LOCK))
	;	/* nothing */
HW_CCM_ANALOG_PLL_USB2_CLR(BM_CCM_ANALOG_PLL_USB2_BYPASS);
HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_ENABLE);

/*
	Reset the Phy
*/
HW_USBPHY_CTRL_CLR(HW_USBPHY2, BM_USBPHY_CTRL_SFTRST);
HW_USBPHY_CTRL_CLR(HW_USBPHY2, BM_USBPHY_CTRL_CLKGATE);
HW_USBPHY_PWD_WR(HW_USBPHY2, 0);

/*
	Who-knows why, we just gotta do this (according to the docs)
*/
HW_USBPHY_TX(HW_USBPHY2).B.TXCAL45DP = 0x00;
HW_USBPHY_TX(HW_USBPHY2).B.TXCAL45DN = 0x00;
HW_USBPHY_TX(HW_USBPHY2).B.D_CAL = 0x07;

/*
	We move on from the USB Phy to the USB controller...
	Reset the USB port
*/
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD()| BM_USBC_UH1_USBCMD_RST);
while (HW_USBC_UH1_USBCMD_RD() & BM_USBC_UH1_USBCMD_RST)
	;	// nothing

/*
	Tell the controller we're a HOST.  Possible values are (see Page 5427 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012")
		00 Idle [Default for combination host/device]
		01 Reserved
		10 Device Controller [Default for device only controller]
		11 Host Controller [Default for host only controller]
*/
HW_USBC_UH1_USBMODE.B.CM = 3;			// HOST mode

/*
	Run and Power-up
*/
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() | BM_USBC_UH1_USBCMD_RS);
HW_USBC_UH1_PORTSC1_WR(HW_USBC_UH1_PORTSC1_RD() | BM_USBC_UH1_PORTSC1_PP);

/*
	Clear the status flag (ignore all interrupts up to this point)
*/
HW_USBC_UH1_USBSTS.U = HW_USBC_UH1_USBSTS.U;

/*
	Enable interrupts
	The Port Change Detect interrupt is essential for a disconnect and reconnect while we're powered up.  If
	we don't enable the port connect then we don't get the reset interrupt to tell us we've just connected to
	a host.

		BM_USBC_UH1_USBINTR_URE |	// USB USB Reset Received
		BM_USBC_UH1_USBINTR_PCE |	// USB Port Change Detect
		BM_USBC_UH1_USBINTR_UE		// USB Interrupt (USBINT)
*/
HW_USBC_UH1_USBINTR_WR(BM_USBC_UH1_USBINTR_URE | BM_USBC_UH1_USBINTR_PCE | BM_USBC_UH1_USBINTR_UE);

/*
	Set the port speed, options are:
		BV_USBC_UOG_PORTSC1_PSPD__FULL  0		// 12Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__LOW   1		// 1.5Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__HIGH  2		// 480Mb/s
*/
HW_USBC_UH1_PORTSC1.B.PSPD = 2;
}

/*
	ATOSE_HOST_USB::GET_INTERRUP_ID()
	---------------------------------
*/
uint32_t ATOSE_host_usb::get_interrup_id(void)
{
return IMX_INT_USBOH3_UH1;
}

/*
	ATOSE_HOST_USB::ENABLE()
	------------------------
*/
void ATOSE_host_usb::enable(void)
{
/*
	Initialise the semaphore
*/
semaphore = ATOSE_atose::get_ATOSE()->process_allocator.malloc_semaphore();
semaphore->clear();
semaphore_handle = (uint32_t)semaphore;

/*
	Initialise the worker thread
*/
uint32_t ATOSE_host_usb_device_manager(void);
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(ATOSE_host_usb_device_manager);

#ifdef SABRE_LITE
	/*
		Turn on the USB Hub on the SABRE Lite board.
		On the SABRE Lite board GPIO port 7/12 is connected to a USB hub that is held in reset until
		the line is set high.  Here we do that
	*/
	SABRE_Lite_hub_reset();
#endif
}

/*
	========================
	========================
	========================
*/

/*
	Although the reference manual allignes transfer descriptors on 32-byte boundaries, the
	i.MX6Q SDK (iMX6_Platform_SDK\sdk\drivers\usb\src\usbh_drv.c) alligns them on 64-byte
	boundaries (see usbh_qtd_init()).
*/
static ATOSE_usb_ehci_queue_element_transfer_descriptor global_qTD1  __attribute__ ((aligned(64)));
static ATOSE_usb_ehci_queue_element_transfer_descriptor global_qTD2  __attribute__ ((aligned(64)));
static ATOSE_usb_ehci_queue_element_transfer_descriptor global_qTD3  __attribute__ ((aligned(64)));

/*
   The i.MX6Q SDK (iMX6_Platform_SDK\sdk\drivers\usb\src\usbh_drv.c) alligns on 64-byte
	boundaries (see usbh_qh_init()), so we do the same here.
*/
static ATOSE_usb_ehci_queue_head queue_head __attribute__ ((aligned(64)));

/*
	ATOSE_HOST_USB::::USB_BUS_RESET()
	---------------------------------
*/
void ATOSE_host_usb::usb_bus_reset(void)
{
/*
	Tell the controller to perform the Port Reset
*/
HW_USBC_UH1_PORTSC1_WR(HW_USBC_UH1_PORTSC1_RD() | BM_USBC_UH1_PORTSC1_PR);

/*
	And wait for it to finish
*/
while(HW_USBC_UH1_PORTSC1_RD() & BM_USBC_UH1_PORTSC1_PR)
	; /* do nothing */
}

/*
	ATOSE_HOST_USB::INITIALISE_QUEUEHEAD()
	--------------------------------------
*/
void ATOSE_host_usb::initialise_queuehead(ATOSE_usb_ehci_queue_head *queue_head, uint32_t device, uint32_t endpoint)
{
/*
	Set everything to zero
*/
memset(queue_head, 0, sizeof(queue_head));

/*
	Create a circular list of just this one queuehead
*/
queue_head->queue_head_horizontal_link_pointer.all = queue_head;		// point to self
queue_head->queue_head_horizontal_link_pointer.bit.typ = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::QUEUE_HEAD;	// we're a queuehead
queue_head->characteristics.bit.h = 1;				// we're the head of the list

/*
	Set the port speed and packet size (which is based on the port speed)
*/
queue_head->characteristics.bit.ep = HW_USBC_UH1_PORTSC1.B.PSPD;	// port speed
queue_head->characteristics.bit.maximum_packet_length = (queue_head->characteristics.bit.ep == 0x01 ? 0x08 : 0x40);		// 0x08 for LOW-speed else 0x40
queue_head->characteristics.bit.dtc = 1;

/*
	We're for a particular endpoint on a particular device (but we probably don't know what those are yet)
*/
queue_head->characteristics.bit.device_address = device;
queue_head->characteristics.bit.endpt = endpoint;

/*
	One transaction per micro-frame
*/
queue_head->capabilities.bit.mult = ATOSE_usb_ehci_queue_head_endpoint_capabilities::TRANSACTIONS_ONE;

/*
	we don't yet have a descriptor so we mark the descriptors as dead-ends
*/
queue_head->next_qtd_pointer = queue_head->alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
}

/*
	ATOSE_HOST_USB::INITIALISE_TRANSFER_DESCRIPTOR()
	------------------------------------------------
	data need not be aligned correctly, that is taken care of by this round.
	the maximum amount of data we can "safely" transmit is (BUFFER_POINTERS - 1) * 4096 + 1
	because if we worst case aligned then the first buffer will contain only one byte.
	So, don't call this routine with more than 20KB of data
*/
void ATOSE_host_usb::initialise_transfer_descriptor(ATOSE_usb_ehci_queue_element_transfer_descriptor *descriptor, uint32_t transaction_type, char *data, uint32_t data_length)
{
uint32_t offset, remaining, block;

/*
	Set everything to zero
*/
memset(descriptor, 0, sizeof(*descriptor));

/*
	Mark the descriptor as being at the end of the queue
*/
descriptor->next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
descriptor->alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;

/*
	Set the transaction type (IN / OUT / SETUP)
*/
descriptor->token.bit.pid_code = transaction_type;
if (transaction_type != ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_SETUP)
	descriptor->token.bit.dt = 1;

/*
	Don't interrupt (normally only the last object in the list will cause an interrupt)
	Mark the descriptor as active and allow 3 errors before it is rejected
*/
descriptor->token.bit.ioc = 0;
descriptor->token.bit.c_err = 3;
descriptor->token.bit.status = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::STATUS_ACTIVE;

/*
	Now on to the data.  The i.MX6Q counts up-to a 4KB boundary so if we don't
	pre-align the data pointer then we must break it on 4KB boundaries.

	This code is (heavily) based on: usbh_qtd_init() from usbh_drv.c included in the i.MX6Q SDK (iMX6_Platform_SDK\sdk\drivers\usb\src)
*/
descriptor->token.bit.total_bytes = data_length;
descriptor->buffer_pointer[0] = data;

offset = ((uint32_t)data) % ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_SIZE;
remaining = data_length - (ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_SIZE - offset);

/*
	All the other buffers start on a ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_SIZE (4KB) boundary
*/
for (block = 1; block < ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_POINTERS; block++)
	if (remaining > 0)
		{
		descriptor->buffer_pointer[block] = (uint8_t *)((uint32_t)data) + ((ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_SIZE * block) - offset);
		remaining -= ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_SIZE;
		}
}

/*
	ATOSE_HOST_USB::ACKNOWLEDGE()
	-----------------------------
*/
void ATOSE_host_usb::acknowledge(ATOSE_registers *registers)
{
hw_usbc_uog_usbsts_t usb_status;

/*
	Acknowledge all the interrupts
*/
usb_status.U = HW_USBC_UH1_USBSTS.U;
HW_USBC_UH1_USBSTS.U = usb_status.U;

/*
	USBINT (as it's know) After initialisation and under normal operation we expect only this case.
*/
if (usb_status.B.UI)
	{
	debug_print_string("USB INT\r\n");
	semaphore->signal();
	}

/*
	USB Reset Interrupt
*/
if (usb_status.B.URI)
	{
	debug_print_string("USB RESET\r\n");
	}

/*
	USB Port Change Interrupt
*/
if (usb_status.B.PCI)
	{
	debug_print_string("USB PCI\r\n");
	/*
		Wait for the connect to finish
	*/
	while(!(HW_USBC_UH1_PORTSC1_RD() & BM_USBC_UH1_PORTSC1_CCS))
		; /* nothing */

	semaphore->signal();
	HW_USBC_UH1_USBSTS.B.PCI = 1;
	}
}

/*
	ATOSE_HOST_USB::SEND_SETUP_PACKET_TO_DEVICE()
	---------------------------------------------
*/
void ATOSE_host_usb::send_setup_packet_to_device(uint32_t device, uint32_t endpoint, ATOSE_usb_setup_data *packet, void *descriptor, uint8_t size)
{
/*
	Initialise the queuehead and bung it in the Async list
*/
initialise_queuehead(&queue_head, device, endpoint);

/*
	Initialise the descriptors
*/
initialise_transfer_descriptor(&global_qTD1, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_SETUP, (char *)packet, sizeof(*packet));
initialise_transfer_descriptor(&global_qTD2, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_IN, (char *)descriptor, size);
global_qTD1.next_qtd_pointer = &global_qTD2;

if (descriptor == NULL)
	global_qTD2.token.bit.ioc = 1;
else
	{
	initialise_transfer_descriptor(&global_qTD3, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_OUT, 0, 0);

	global_qTD2.next_qtd_pointer = &global_qTD3;
	global_qTD3.token.bit.ioc = 1;
	}

/*
	Shove the descriptors into the queuehead
*/
queue_head.next_qtd_pointer = &global_qTD1;

/*
	Pass the queuehead on the to the i.MX6Q
*/
HW_USBC_UH1_ASYNCLISTADDR_WR((uint32_t)&queue_head);

/*
	Enable the Async list and wait for it to start
*/
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() | BM_USBC_UH1_USBCMD_ASE);
while(!(HW_USBC_UH1_USBSTS_RD() & BM_USBC_UH1_USBSTS_AS))
	;	/* do nothing */

debug_print_string("WAITONSEND\r\n");
ATOSE_api::semaphore_wait(semaphore_handle);
}

/*
	ATOSE_HOST_USB_DEVICE_MANAGER()
	-------------------------------
*/
uint32_t ATOSE_host_usb_device_manager(void)
{
ATOSE_atose::get_ATOSE()->imx6q_host_usb.device_manager();

while (1);
return 0;
}

/*
	ATOSE_HOST_USB::WAIT_FOR_CONNECTION()
	-------------------------------------
*/
void ATOSE_host_usb::wait_for_connection(void)
{
/*
	Wait for a connect event
*/
debug_print_string("WAITONCONNECTION\r\n");
ATOSE_api::semaphore_wait(semaphore_handle);

/*
	Reset the USB bus
*/
usb_bus_reset();
ATOSE_api::semaphore_wait(semaphore_handle);
}

/*
	ATOSE_HOST_USB::GET_DEVICE_DESCRIPTOR()
	---------------------------------------
*/
ATOSE_usb_standard_device_descriptor *ATOSE_host_usb::get_device_descriptor(ATOSE_usb_standard_device_descriptor *descriptor)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = 0x80;
setup_packet.bRequest = ATOSE_usb::REQUEST_GET_DESCRIPTOR;
setup_packet.wValue_high = ATOSE_usb::DESCRIPTOR_TYPE_DEVICE;
setup_packet.wValue_low = 0;
setup_packet.wIndex = 0;
setup_packet.wLength = 0x12;

/*
	The first time we send a setup packet we don't know if the device will return just the first "packet" or whether it will
	return the whole descriptor so we ask twice.
*/
send_setup_packet_to_device(0, 0, &setup_packet, descriptor, sizeof(*descriptor));
setup_packet.wLength = sizeof(*descriptor) <= descriptor->bLength ? sizeof(*descriptor) : descriptor->bLength;
send_setup_packet_to_device(0, 0, &setup_packet, descriptor, sizeof(*descriptor));

return descriptor;
}

/*
	ATOSE_HOST_USB::GET_CONFIGURATION_DESCRIPTOR()
	----------------------------------------------
*/
ATOSE_usb_standard_configuration_descriptor *ATOSE_host_usb::get_configuration_descriptor(uint32_t address, ATOSE_usb_standard_configuration_descriptor *descriptor, uint32_t size)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = 0x80;
setup_packet.bRequest = ATOSE_usb::REQUEST_GET_DESCRIPTOR;
setup_packet.wValue_high = ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION;
setup_packet.wValue_low = 0;
setup_packet.wIndex = 0;
setup_packet.wLength = size == 0 ? sizeof(*descriptor) : size;

send_setup_packet_to_device(address, 0, &setup_packet, descriptor, setup_packet.wLength);

return descriptor;
}

/*
	ATOSE_HOST_USB::SET_ADDRESS()
	-----------------------------
*/
void ATOSE_host_usb::set_address(uint32_t address)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = 0x00;
setup_packet.bRequest = ATOSE_usb::REQUEST_SET_ADDRESS;
setup_packet.wValue = address;
setup_packet.wIndex = 0;
setup_packet.wLength = 0;

send_setup_packet_to_device(0, 0, &setup_packet, 0, 0);
}

/*
	ATOSE_HOST_USB::SET_CONFIGURATION()
	-----------------------------------
*/
void ATOSE_host_usb::set_configuration(uint32_t address, uint32_t configuration)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = 0x00;
setup_packet.bRequest = ATOSE_usb::REQUEST_SET_CONFIGURATION;
setup_packet.wValue = configuration;
setup_packet.wIndex = 0;
setup_packet.wLength = 0;

send_setup_packet_to_device(address, 0, &setup_packet, 0, 0);
}

/*
	ATOSE_HOST_USB::DEVICE_MANAGER()
	--------------------------------
*/
void ATOSE_host_usb::device_manager(void)
{
ATOSE_usb_standard_device_descriptor device_descriptor;
ATOSE_usb_standard_configuration_descriptor configuration_descriptor;

wait_for_connection();
get_device_descriptor(&device_descriptor);

/*
	Dump the descriptor
*/
debug_print_string("DEVICE DESCRIPTOR\r\n-----------------\r\n");
debug_print_this("bLength            :", device_descriptor.bLength);
debug_print_this("bDescriptorType    :", device_descriptor.bDescriptorType);
debug_print_this("bcdUSB             :", device_descriptor.bcdUSB);
debug_print_this("bDeviceClass       :", device_descriptor.bDeviceClass);
debug_print_this("bDeviceSubClass    :", device_descriptor.bDeviceSubClass);
debug_print_this("bDeviceProtocol    :", device_descriptor.bDeviceProtocol);
debug_print_this("bMaxPacketSize0    :", device_descriptor.bMaxPacketSize0);
debug_print_this("idVendor           :", device_descriptor.idVendor);
debug_print_this("idProduct          :", device_descriptor.idProduct);
debug_print_this("bcdDevice          :", device_descriptor.bcdDevice);
debug_print_this("iManufacturer      :", device_descriptor.iManufacturer);
debug_print_this("iProduct           :", device_descriptor.iProduct);
debug_print_this("iSerialNumber      :", device_descriptor.iSerialNumber);
debug_print_this("bNumConfigurations :", device_descriptor.bNumConfigurations);

/*
	We know about some classes of device including USB Hubs
*/
if (device_descriptor.bDeviceClass == ATOSE_usb::DEVICE_CLASS_HUB)
	{
	/*
		We're a USB Hub
	*/
	debug_print_string("Set Address\r\n");
	set_address(3);

	debug_print_string("GET CONFIG DESCRIPTOR\r\n");
	get_configuration_descriptor(3, &configuration_descriptor);

	debug_print_this("bLength            :", configuration_descriptor.bLength);
	debug_print_this("bDescriptorType    :", configuration_descriptor.bDescriptorType);
	debug_print_this("wTotalLength       :", configuration_descriptor.wTotalLength);
	debug_print_this("bNumInterfaces     :", configuration_descriptor.bNumInterfaces);
	debug_print_this("bConfigurationValue:", configuration_descriptor.bConfigurationValue);
	debug_print_this("iConfiguration     :", configuration_descriptor.iConfiguration);
	debug_print_this("bmAttributes       :", configuration_descriptor.bmAttributes);
	debug_print_this("bMaxPower          :", configuration_descriptor.bMaxPower);

	debug_print_string("SET CONFIGURATION\r\n");
	set_configuration(3, 1);

	static uint8_t buffer[1024];
	debug_print_string("GET_CONFIGURATION_DESCRIPTOR\r\n");
	get_configuration_descriptor(3, (ATOSE_usb_standard_configuration_descriptor *)buffer, configuration_descriptor.wTotalLength);
	void dump_configuration_descriptor(uint8_t *descriptor);
	dump_configuration_descriptor(buffer);
	}
}

/*
	DUMP_CONFIGURATION_DESCRIPTOR()
	-------------------------------
*/
void dump_configuration_descriptor(uint8_t *descriptor)
{
ATOSE_usb_standard_endpoint_descriptor *endpoint_descriptor;
ATOSE_usb_standard_interface_descriptor *interface_descriptor;
ATOSE_usb_standard_configuration_descriptor *configuration_descriptor;
uint8_t *end;

configuration_descriptor = (ATOSE_usb_standard_configuration_descriptor *)descriptor;
end = descriptor + configuration_descriptor->wTotalLength;
while (descriptor < end)
	{
	configuration_descriptor = (ATOSE_usb_standard_configuration_descriptor *)descriptor;
	descriptor += configuration_descriptor->bLength;
	if (configuration_descriptor->bDescriptorType == ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION)
		{
		debug_print_string("CONFIGURATION\r\n");
		debug_print_this("bLength            :", configuration_descriptor->bLength);
		debug_print_this("bDescriptorType    :", configuration_descriptor->bDescriptorType);
		debug_print_this("wTotalLength       :", configuration_descriptor->wTotalLength);
		debug_print_this("bNumInterfaces     :", configuration_descriptor->bNumInterfaces);
		debug_print_this("bConfigurationValue:", configuration_descriptor->bConfigurationValue);
		debug_print_this("iConfiguration     :", configuration_descriptor->iConfiguration);
		debug_print_this("bmAttributes       :", configuration_descriptor->bmAttributes);
		debug_print_this("bMaxPower          :", configuration_descriptor->bMaxPower);
		}
	else if (configuration_descriptor->bDescriptorType == ATOSE_usb::DESCRIPTOR_TYPE_INTERFACE)
		{
		interface_descriptor = (ATOSE_usb_standard_interface_descriptor *)configuration_descriptor;

		debug_print_string("INTERFACE\r\n");
		debug_print_this("bLength            :", interface_descriptor->bLength);
		debug_print_this("bDescriptorType    :", interface_descriptor->bDescriptorType);
		debug_print_this("bInterfaceNumber   :", interface_descriptor->bInterfaceNumber);
		debug_print_this("bAlternateSetting  :", interface_descriptor->bAlternateSetting);
		debug_print_this("bNumEndpoints      :", interface_descriptor->bNumEndpoints);
		debug_print_this("bInterfaceClass    :", interface_descriptor->bInterfaceClass);
		debug_print_this("bInterfaceSubClass :", interface_descriptor->bInterfaceSubClass);
		debug_print_this("bInterfaceProtocol :", interface_descriptor->bInterfaceProtocol);
		debug_print_this("iInterface         :", interface_descriptor->iInterface);
		}

	else if (configuration_descriptor->bDescriptorType == ATOSE_usb::DESCRIPTOR_TYPE_ENDPOINT)
		{
		endpoint_descriptor = (ATOSE_usb_standard_endpoint_descriptor *)configuration_descriptor;

		debug_print_string("ENDPOINT\r\n");
		debug_print_this("bLength            :", endpoint_descriptor->bLength);
		debug_print_this("bDescriptorType    :", endpoint_descriptor->bDescriptorType);
		debug_print_this("bEndpointAddress   :", endpoint_descriptor->bEndpointAddress);
		debug_print_this("bmAttributes       :", endpoint_descriptor->bmAttributes);
		debug_print_this("wMaxPacketSize     :", endpoint_descriptor->wMaxPacketSize);
		debug_print_this("bInterval          :", endpoint_descriptor->bInterval);
		}

	else
		{
		debug_print_string("UNKNOWN\r\n");
		debug_print_this("bLength            :", configuration_descriptor->bLength);
		debug_print_this("bDescriptorType    :", configuration_descriptor->bDescriptorType);
		return;
		}
	}
}

