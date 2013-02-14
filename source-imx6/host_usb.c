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
#include "host_usb.h"
#include "ascii_str.h"

#include "usb_ehci_queue_head.h"
#include "usb_standard_device_descriptor.h"
#include "usb_ehci_queue_head_endpoint_capabilities.h"
#include "usb_ehci_queue_element_transfer_descriptor.h"
#include "usb_ehci_queue_head_horizontal_link_pointer.h"
#include "usb_ehci_queue_head_endpoint_characteristics.h"

/*
	=====================================================
	=====================================================
	=====================================================
*/
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"

#define DEFAULT_UART 2

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

/*
	=====================================================
	=====================================================
	=====================================================
*/

#define DEFAULT_TIMER 1

/*
	DELAY_INIT()
	------------
*/
void delay_init(void)
{
uint32_t speed_in_Hz[] = {528000000, 396000000, 352000000, 198000000, 594000000};
uint32_t frequency;

HW_CCM_CCGR1.B.CG6 = 0x03;

HW_EPIT_CR_WR(DEFAULT_TIMER, BM_EPIT_CR_SWR);
while ((HW_EPIT_CR(DEFAULT_TIMER).B.SWR) != 0)
	;	// nothing

frequency = speed_in_Hz[HW_CCM_CBCMR.B.PRE_PERIPH_CLK_SEL] / (HW_CCM_CBCDR.B.AHB_PODF + 1) / (HW_CCM_CBCDR.B.IPG_PODF + 1);
HW_EPIT_CR_WR(DEFAULT_TIMER, BF_EPIT_CR_CLKSRC(1) | BF_EPIT_CR_PRESCALAR((frequency / 1000000) - 1) | BM_EPIT_CR_RLD | BM_EPIT_CR_IOVW | BM_EPIT_CR_ENMOD);
}

/*
	DELAY_US()
	----------
*/
void delay_us(uint32_t time_in_us)
{
HW_EPIT_LR_WR(DEFAULT_TIMER, time_in_us);
HW_EPIT_SR_SET(DEFAULT_TIMER, BM_EPIT_SR_OCIF);
HW_EPIT_CR_SET(DEFAULT_TIMER, BM_EPIT_CR_EN);

while (HW_EPIT_SR_RD(DEFAULT_TIMER) == 0)
	;	// nothing (i.e. wait)

HW_EPIT_CR_CLR(DEFAULT_TIMER, BM_EPIT_CR_EN);
}

/*
	=====================================================
	=====================================================
	=====================================================
*/

// Function to configure IOMUXC for gpio7 module.
/*
	SABRE_LITE_HUB_RESET()
	----------------------
	The SABRE Lite board has a 3-port USB hub connected to USB Port 1.  If we want to connect to a device to the
	board its going to go through this hub and so we need to reset the hub.  The USB Hub is connected to pin 12 of
	GPIO port 7.
*/
void SABRE_Lite_hub_reset(void)
{
delay_init();

/*
	Enable the pads (this code was generated by the IOMUX tool)
*/
HW_IOMUXC_SW_MUX_CTL_PAD_GPIO17_WR(BF_IOMUXC_SW_MUX_CTL_PAD_GPIO17_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_GPIO17_MUX_MODE_V(ALT5));
HW_IOMUXC_SW_PAD_CTL_PAD_GPIO17_WR(BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_GPIO17_SRE_V(SLOW));

/*
	Set the direction to output.  The Hub is connected to pin 12 of GPIO port 7
*/
HW_GPIO_GDIR_SET(7, 1 << 12);
delay_us(20);		// I think we have to wait 2 "wait states", but I'm not sure how long that is.  20us seems to work

/*
	Hold the line high (in the 1-state)
*/
HW_GPIO_DR_SET(7, 1 << 12);
}

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
	We want to know about the following events:
		BM_USBC_UH1_USBSTS_URI |	// USB USB Reset Received
		BM_USBC_UH1_USBSTS_PCI |	// USB Port Change Detect
		BM_USBC_UH1_USBSTS_UI		// USB Interrupt (USBINT)

*/
HW_USBC_UH1_USBSTS_SET(
	BM_USBC_UH1_USBSTS_AAI | 		// Async Advance Interrupt Enable
	BM_USBC_UH1_USBSTS_SEI | 		// ERROR interrupt
	BM_USBC_UH1_USBSTS_UEI | 		// ERROR interrupt
	BM_USBC_UH1_USBSTS_URI |
	BM_USBC_UH1_USBSTS_PCI |
	BM_USBC_UH1_USBSTS_UI);

/*
	Enable interrupts
	The Port Change Detect interrupt is essential for a disconnect and reconnect while we're powered up.  If
	we don't enable the port connect then we don't get the reset interrupt to tell us we've just connected to
	a host.
*/
HW_USBC_UH1_USBINTR_WR(
	BM_USBC_UH1_USBINTR_AAE | 		// Async Advance Interrupt Enable
	BM_USBC_UH1_USBINTR_SEE | 		// ERROR interrupt
	BM_USBC_UH1_USBINTR_UEE | 		// ERROR interrupt
	BM_USBC_UH1_USBINTR_URE |
	BM_USBC_UH1_USBINTR_PCE |
	BM_USBC_UH1_USBINTR_UE
);

/*
	Set the port speed, options are:
		BV_USBC_UOG_PORTSC1_PSPD__FULL  0		// 12Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__LOW   1		// 1.5Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__HIGH  2		// 480Mb/s
*/
HW_USBC_UH1_PORTSC1.B.PSPD = 2;

/*
	Turn on the USB Hub on the SABRE Lite board.
	On the SABRE Lite board GPIO port 7/12 is connected to a USB hub that is held in reset until
	the line is set high.  Here we do that
*/
SABRE_Lite_hub_reset();
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
}

/*
	========================
	========================
	========================
*/

static ATOSE_usb_ehci_queue_head queue_head __attribute__ ((aligned(32)));
static ATOSE_usb_ehci_queue_element_transfer_descriptor global_qTD  __attribute__ ((aligned(64)));
static uint8_t buffer[4096] __attribute__ ((aligned(4096)));

/*
	SEND_SETUP_PACKET_TO_DEVICE()
	-----------------------------
*/
void send_setup_packet_to_device(uint32_t device, uint32_t endpoint, ATOSE_usb_setup_data *packet)
{
memset(&queue_head, 0, sizeof(queue_head));
queue_head.queue_head_horizontal_link_pointer.all = (uint32_t)&queue_head;																// point to self
queue_head.queue_head_horizontal_link_pointer.bit.typ = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::QUEUE_HEAD;	// we're a queuehead
queue_head.queue_head_horizontal_link_pointer.bit.t = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::TERMINATOR;		// and we're the last one

queue_head.characteristics.bit.c = 1;					// we're a control endpoint
queue_head.characteristics.bit.maximum_packet_length = 8;		// MAX_PACKET_SIZE
queue_head.characteristics.bit.ep = ATOSE_usb_ehci_queue_head_endpoint_characteristics::SPEED_LOW;
queue_head.characteristics.bit.endpt = endpoint;
queue_head.characteristics.bit.device_address = device;		// it doesn't have an address yet.

queue_head.capabilities.bit.mult = ATOSE_usb_ehci_queue_head_endpoint_capabilities::TRANSACTIONS_ONE;

queue_head.next_qtd_pointer = &global_qTD;
queue_head.alternate_next_qtd_pointer = &global_qTD;

global_qTD.next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
global_qTD.alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;

global_qTD.token.bit.dt = 0;
global_qTD.token.bit.total_bytes = sizeof(*packet);
global_qTD.token.bit.ioc = 1;
global_qTD.token.bit.c_page = 0;
global_qTD.token.bit.c_err = 1;
global_qTD.token.bit.pid_code = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_SETUP;
global_qTD.token.bit.status = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::STATUS_ACTIVE;

memcpy(buffer, packet, sizeof(*packet));
global_qTD.buffer_pointer[0] = buffer;

debug_print_string("\r\n");
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");

HW_USBC_UH1_ASYNCLISTADDR.B.ASYBASE = (uint32_t)&queue_head;
HW_USBC_UH1_USBCMD.B.ASE = 1;

debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");
delay_us(100000);
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");


//delay_us(1000);
HW_USBC_UH1_USBCMD.B.ASE = 0;
HW_USBC_UH1_USBSTS.U = HW_USBC_UH1_USBSTS.U;
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");

}

/*
	READ_FROM_DEVICE()
	------------------
*/
void read_from_device(uint32_t device, uint32_t endpoint, uint8_t *parametric_buffer, uint32_t parametric_buffer_length)
{
memset(&queue_head, 0, sizeof(queue_head));
queue_head.queue_head_horizontal_link_pointer.all = (uint32_t)&queue_head.queue_head_horizontal_link_pointer;				// point to self
queue_head.queue_head_horizontal_link_pointer.bit.typ = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::QUEUE_HEAD;	// we're a queuehead
queue_head.queue_head_horizontal_link_pointer.bit.t = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::TERMINATOR;		// and we're the last one

queue_head.characteristics.bit.device_address = device;		// it doesn't have an address yet.
queue_head.characteristics.bit.endpt = endpoint;
queue_head.characteristics.bit.maximum_packet_length = 8;		// MAX_PACKET_SIZE
queue_head.characteristics.bit.ep = ATOSE_usb_ehci_queue_head_endpoint_characteristics::SPEED_HIGH;

queue_head.capabilities.bit.mult = ATOSE_usb_ehci_queue_head_endpoint_capabilities::TRANSACTIONS_ONE;

queue_head.next_qtd_pointer = &global_qTD;

global_qTD.next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
global_qTD.alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;

global_qTD.token.bit.status = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::STATUS_ACTIVE;
global_qTD.token.bit.pid_code = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_IN;
global_qTD.token.bit.c_err = 0;
global_qTD.token.bit.c_page = 0;
global_qTD.token.bit.ioc = 1;
global_qTD.token.bit.total_bytes = parametric_buffer_length;
global_qTD.token.bit.dt = 0;

global_qTD.buffer_pointer[0] = buffer;

debug_print_string("\r\nREC:\r\n");
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");
HW_USBC_UH1_ASYNCLISTADDR.B.ASYBASE = (uint32_t)&queue_head;
HW_USBC_UH1_USBCMD.B.ASE = 1;
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");
delay_us(100000);
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");
HW_USBC_UH1_USBCMD.B.ASE = 0;
debug_print_this("HW_USBC_UH1_USBSTS :", HW_USBC_UH1_USBSTS.U, "");

memcpy(parametric_buffer, buffer, parametric_buffer_length);
}

/*
	ATOSE_HOST_USB::ACKNOWLEDGE()
	-----------------------------
*/
void ATOSE_host_usb::acknowledge(void)
{
hw_usbc_uog_usbsts_t usb_status;

usb_status.U = HW_USBC_UH1_USBSTS.U;
HW_USBC_UH1_USBSTS.U = usb_status.U;

debug_print_string("[HW_USBC_UH1_USBSTS:");
debug_print_hex(usb_status.U);

/*
	After initialisation and under normal operation we expect only this case.
*/
if (usb_status.B.UI)
	{
	debug_print_string("USB INTERRUPT]\r\n");
	}

else if (usb_status.B.UI)
	{
	debug_print_string("USB ERROR]\r\n");
	}
/*
	USB Reset Interrupt
*/
else if (usb_status.B.URI)
	{
	debug_print_string("USB RESET]\r\n");
	}

/*
	USB Port Change Interrupt
	If we disable this then when the user disconnects and reconnects then we don't get a reset message!!!
*/
else if (usb_status.B.PCI)
	{
	debug_print_string("USB PCI:");
	/*
		Page 5223 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
		"To communicate
		with devices through the asynchronous schedule, system software must write the
		USB_ASYNCLISTADDR register with the address of a control or bulk queue head.
		Software must then enable the asynchronous schedule by writing one to the
		Asynchronous Schedule Enable bit in the USB_USBCMD register. To communicate with
		devices through the periodic schedule, system software must enable the periodic schedule
		by writing one to the Periodic Schedule Enable bit in the USB_USBCMD register."
	*/

	/*
		The first request should be a setup packet asking for the device descriptor.
	*/
	ATOSE_usb_setup_data setup_packet;
	ATOSE_usb_standard_device_descriptor descriptor;

	setup_packet.bmRequestType.all = 0x80;
	setup_packet.bRequest = ATOSE_usb::REQUEST_GET_DESCRIPTOR;
	setup_packet.wValue = ATOSE_usb::DESCRIPTOR_TYPE_DEVICE;
	setup_packet.wIndex = 0;
	setup_packet.wLength = 8;

	memset(buffer, 0xFF, sizeof(buffer));
	send_setup_packet_to_device(0, 0, &setup_packet);

	delay_us(1000 * 1000);	// wait 1 second
	debug_print_this("qTD.status:", global_qTD.token.bit.status);


	memset(buffer, 0xFF, sizeof(buffer));
	read_from_device(0, 0, (uint8_t *)&descriptor, 8);

	debug_print_string("--\r\n");
	debug_print_this("bLength         :", descriptor.bLength);
	debug_print_this("bDescriptorType :", descriptor.bDescriptorType);
	debug_print_this("bcdUSB          :", descriptor.bcdUSB);
	debug_print_this("bDeviceClass    :", descriptor.bDeviceClass);
	debug_print_this("bDeviceSubClass :", descriptor.bDeviceSubClass);
	debug_print_this("bDeviceProtocol :", descriptor.bDeviceProtocol);
	debug_print_this("bMaxPacketSize0 :", descriptor.bMaxPacketSize0);

	setup_packet.bmRequestType.all = 0x80;
	setup_packet.bRequest = ATOSE_usb::REQUEST_GET_DESCRIPTOR;
	setup_packet.wValue = ATOSE_usb::DESCRIPTOR_TYPE_DEVICE;
	setup_packet.wIndex = 0;
	setup_packet.wLength = sizeof(descriptor) <= descriptor.bLength ? sizeof(descriptor) :  descriptor.bLength;

	memset(buffer, 0xFF, sizeof(buffer));
	memset(&descriptor, 0xFF, sizeof(descriptor));
	send_setup_packet_to_device(0, 0, &setup_packet);
	memset(buffer, 0xFF, sizeof(buffer));
	read_from_device(0, 0, (uint8_t *)&descriptor, setup_packet.wLength);

	debug_print_string("--\r\n");
	debug_print_this("bLength         :", descriptor.bLength);
	debug_print_this("bDescriptorType :", descriptor.bDescriptorType);
	debug_print_this("bcdUSB          :", descriptor.bcdUSB);
	debug_print_this("bDeviceClass    :", descriptor.bDeviceClass);
	debug_print_this("bDeviceSubClass :", descriptor.bDeviceSubClass);
	debug_print_this("bDeviceProtocol :", descriptor.bDeviceProtocol);
	debug_print_this("bMaxPacketSize0 :", descriptor.bMaxPacketSize0);
	debug_print_this("idVendor :", descriptor.idVendor);
	debug_print_this("idProduct :", descriptor.idProduct);
	debug_print_this("bcdDevice :", descriptor.bcdDevice);
	debug_print_this("iManufacturer :", descriptor.iManufacturer);
	debug_print_this("iProduct :", descriptor.iProduct);
	debug_print_this("iSerialNumber :", descriptor.iSerialNumber);
	debug_print_this("bNumConfigurations :", descriptor.bNumConfigurations);

	debug_print_string("]\r\n");
	}
else
	{
	debug_print_string("USB Unknown Unterrupt]\r\n");
	}
}
