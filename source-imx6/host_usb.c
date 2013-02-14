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

void do_some_magic(void);

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
	DEBIUG_DUMP_BUFFER()
	--------------------
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

delay_us(1000);		// I think we have to wait 2 "wait states", but I'm not sure how long that is. 20us seems to work

/*
	Hold the line high (in the 1-state)
*/
HW_GPIO_DR_SET(7, 1 << 12);
delay_us(1000);
}

/*
	=====================================================
	=====================================================
	=====================================================
*/
/*
	VOID USBENABLECLOCKS(VOID)
	--------------------------
*/
void usbEnableClocks(void)
{
/*
* Enable the USB clock for the controller
*/
HW_CCM_CCGR6.B.CG0 = 3;

/*
* Enable the PLL\n
* OTG, Host2 and Host3 controllers use USB_PLL0
* Host1 controller uses USB_PLL1
*/

HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_POWER);
HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_EN_USB_CLKS);

while(!(HW_CCM_ANALOG_PLL_USB2_RD() & BM_CCM_ANALOG_PLL_USB2_LOCK))
	;// nothing

HW_CCM_ANALOG_PLL_USB2_CLR(BM_CCM_ANALOG_PLL_USB2_BYPASS);
HW_CCM_ANALOG_PLL_USB2_SET(BM_CCM_ANALOG_PLL_USB2_ENABLE);

}
/*
	VOID USBENABLETRANSCEIVER(VOID)
	-------------------------------
*/
void usbEnableTransceiver(void)
{
uint32_t instance;

instance = HW_USBPHY2;

//! NOTE !! CLKGATE must be cleared before clearing power down
HW_USBPHY_CTRL_CLR(instance, BM_USBPHY_CTRL_SFTRST);	//! - clear SFTRST
HW_USBPHY_CTRL_CLR(instance, BM_USBPHY_CTRL_CLKGATE);	//! - clear CLKGATE
HW_USBPHY_PWD_WR(instance, 0);	//! - clear all power down bits
HW_USBPHY_CTRL_SET(instance, BM_USBPHY_CTRL_ENUTMILEVEL2 | BM_USBPHY_CTRL_ENUTMILEVEL3 | BM_USBPHY_CTRL_ENHOSTDISCONDETECT);

//! disable the charger detector. This must be off during normal operation
	{
	// this register is not documented. Will be updated in the next release
	uint32_t *ChargerDetectControl;
	ChargerDetectControl = (uint32_t *) 0x020c81b0;
	*ChargerDetectControl |= 1 << 20;   // disable detector
	}
}

int32_t gpio_set_direction(int32_t port, int32_t pin, int32_t dir)
{
uint32_t oldVal = 0, newVal = 0;

if ((port > HW_GPIO_INSTANCE_COUNT) || (port < 1))
	return INVALID_PARAMETER;

if ((pin > 31) || (pin < 0))
	return INVALID_PARAMETER;

oldVal = HW_GPIO_GDIR_RD(port);

if (dir == GPIO_GDIR_INPUT)
	newVal = oldVal & (~(1 << pin));
else
	newVal = oldVal | (1 << pin);

HW_GPIO_GDIR_WR(port, newVal);

return 0; //SUCCESS;
}

int32_t gpio_set_level(int32_t port, int32_t pin, uint32_t level)
{
if ((port > HW_GPIO_INSTANCE_COUNT) || (port < 1))
	return INVALID_PARAMETER;

if ((pin > 31) || (pin < 0))
	return INVALID_PARAMETER;

uint32_t mask = 1 << pin;

int32_t dir = HW_GPIO_GDIR_RD(port) & mask ? GPIO_GDIR_OUTPUT : GPIO_GDIR_INPUT;

if (dir != GPIO_GDIR_OUTPUT)
	return -1;

uint32_t value = HW_GPIO_DR_RD(port);   // read current value

if (level == GPIO_LOW_LEVEL)            // fix it up
	value &= ~mask;
else if ( level == GPIO_HIGH_LEVEL)
	value |= mask;

HW_GPIO_DR_WR(port, value);             // write new value

return 0; //SUCCESS;
}

/*
	VOID USBENABLEVBUS(VOID)
	------------------------
*/
void usbEnableVbus(void)
{
#define reg32_write(addr,val) *((volatile uint32_t *)(addr)) = (val)

reg32_write(IOMUXC_SW_MUX_CTL_PAD_EIM_D30, ALT5);
gpio_set_direction(GPIO_PORT3, 30, GPIO_GDIR_OUTPUT);
gpio_set_level(GPIO_PORT3, 30, GPIO_HIGH_LEVEL);
}

/*
	VOID USBH_INIT(VOID)
	--------------------
*/
void usbh_init(void)
{
usbEnableClocks();
usbEnableTransceiver();

HW_USBC_UH1_PORTSC1_WR(HW_USBC_UH1_PORTSC1_RD() & (~(BF_USBC_UH1_PORTSC1_PTS_1(3) | BF_USBC_UH1_PORTSC1_PTS_2(1))));

//! Reset controller after switching PHY's
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() | BM_USBC_UH1_USBCMD_RST);

//! wait for reset to complete
while (HW_USBC_UH1_USBCMD_RD() & BM_USBC_UH1_USBCMD_RST)
	;/* nothing */

//! set controller to host mode
#define USB_USBMODE_CM_HOST              (0x3)
HW_USBC_UH1_USBMODE_WR(USB_USBMODE_CM_HOST);

//! Set Asynchronous schedule park mode to 3
//! to allow up to 3 successive transactions same queue.
//! set interrupt interval to 0 for immediate interrupt
HW_USBC_UH1_USBCMD_WR(BF_USBC_UH1_USBCMD_ASP(3) | BF_USBC_UH1_USBCMD_ITC(0));

#ifdef USB_USE_INT
/* setup interrupt */
usb_init_host_interrupts(port->moduleBaseAddress);
#endif

//! start the controller
//! the controller will start running but the schedules are not yet enabled.
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() | BM_USBC_UH1_USBCMD_RS);

//! Enable port power.
//! Port power must be set for port to detect a device connection
HW_USBC_UH1_PORTSC1_WR(HW_USBC_UH1_PORTSC1_RD() | BM_USBC_UH1_PORTSC1_PP);

//! Enable Vbus power when Vbus power is controlled by GPIO
//! On some board hardware, Vbus is not controlled by PortPower
//
usbEnableVbus();
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
#ifdef NEVER
/*
	Enable the pads (this code was generated by the IOMUX tool)
*/
HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA30_MUX_MODE_V(ALT6));
HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA30_SRE_V(SLOW));
HW_IOMUXC_USB_H1_OC_SELECT_INPUT_WR(BF_IOMUXC_USB_H1_OC_SELECT_INPUT_DAISY_V(EIM_DATA30_ALT6));
#endif

#ifdef NEVER
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
#else
	usbh_init();
#endif


/*
	Turn on the USB Hub on the SABRE Lite board.
	On the SABRE Lite board GPIO port 7/12 is connected to a USB hub that is held in reset until
	the line is set high.  Here we do that
*/
SABRE_Lite_hub_reset();

do_some_magic();
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
	USB_BUS_RESET()
	---------------
*/
void usb_bus_reset(void)
{
HW_USBC_UH1_PORTSC1_WR(HW_USBC_UH1_PORTSC1_RD() | BM_USBC_UH1_PORTSC1_PR);

//! Wait for reset to finish
while(HW_USBC_UH1_PORTSC1_RD() & BM_USBC_UH1_PORTSC1_PR)
	; /* do nothing */
}
/*
	INIT_QUEUEHEAD()
	----------------
*/
void init_queuehead(ATOSE_usb_ehci_queue_head *queue_head, uint32_t device, uint32_t endpoint)
{
memset(queue_head, 0, sizeof(queue_head));

queue_head->queue_head_horizontal_link_pointer.all = (uint32_t)&queue_head;																// point to self
queue_head->queue_head_horizontal_link_pointer.bit.typ = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::QUEUE_HEAD;	// we're a queuehead
//queue_head->queue_head_horizontal_link_pointer.bit.t = ATOSE_usb_ehci_queue_head_horizontal_link_pointer::TERMINATOR;		// and we're the last one

queue_head->characteristics.bit.ep = HW_USBC_UH1_PORTSC1.B.PSPD;	// port speed
queue_head->characteristics.bit.h = 1;				// we're the head of the list
queue_head->characteristics.bit.maximum_packet_length = 8;		// MAX_PACKET_SIZE
queue_head->characteristics.bit.dtc = 1;
queue_head->characteristics.bit.endpt = endpoint;
queue_head->characteristics.bit.device_address = device;		// it doesn't have an address yet.

queue_head->capabilities.bit.mult = ATOSE_usb_ehci_queue_head_endpoint_capabilities::TRANSACTIONS_ONE;

queue_head->next_qtd_pointer = queue_head->alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
}

/*
	INIT_TRANSFER_DESCRIPTOR()
	--------------------------
	data need not be aligned correctly, that is taken care of by this round.
	the maximum amount of data we can "safely" transmit is (BUFFER_POINTERS - 1) * 4096 + 1
	because if we worst case aligned then the first buffer will contain only one byte.
	So, don't call this routine with more than 20KB of data
*/
void init_transfer_descriptor(ATOSE_usb_ehci_queue_element_transfer_descriptor *descriptor, uint32_t transaction_type, char *data, uint32_t data_length)
{
uint32_t offset, remaining, block;

memset(descriptor, 0, sizeof(*descriptor));

descriptor->next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;
descriptor->alternate_next_qtd_pointer = (ATOSE_usb_ehci_queue_element_transfer_descriptor *)ATOSE_usb_ehci_queue_element_transfer_descriptor::TERMINATOR;

descriptor->token.bit.pid_code = transaction_type;
if (transaction_type != ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_SETUP)
	descriptor->token.bit.dt = 1;
descriptor->token.bit.ioc = 0;
descriptor->token.bit.total_bytes = data_length;
descriptor->token.bit.c_err = 3;
descriptor->token.bit.status = ATOSE_usb_ehci_queue_element_transfer_descriptor_token::STATUS_ACTIVE;

/*
	Now on to the data.  The i.MX6Q counts up-to a 4KB boundary so if we don't
	pre-align the data pointer then we must break it on 4KB boundaries.
*/
descriptor->buffer_pointer[0] = data;

offset = ((uint32_t)data) % 4096;
remaining = data_length - (4096 - offset);

/*
	all the other buffers start on a 4K boundary
*/
for (block = 1; block < ATOSE_usb_ehci_queue_element_transfer_descriptor::BUFFER_POINTERS; block++)
	if (remaining > 0)
		{
		descriptor->buffer_pointer[block] = (uint8_t *)((uint32_t)data) + ((4096 * block) - offset);
		remaining -= 4096;
		}
}

/*
	SEND_SETUP_PACKET_TO_DEVICE()
	-----------------------------
*/
void send_setup_packet_to_device(uint32_t device, uint32_t endpoint, ATOSE_usb_setup_data *packet, ATOSE_usb_standard_device_descriptor *descriptor)
{
init_queuehead(&queue_head, 0, 0);
init_transfer_descriptor(&global_qTD1, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_SETUP, (char *)packet, sizeof(*packet));
init_transfer_descriptor(&global_qTD2, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_IN, (char *)descriptor, sizeof(*descriptor));
init_transfer_descriptor(&global_qTD3, ATOSE_usb_ehci_queue_element_transfer_descriptor_token::PID_OUT, 0, 0);

/*
	Create the chain and interrupt on completion
*/
global_qTD1.next_qtd_pointer = &global_qTD2;
global_qTD2.next_qtd_pointer = &global_qTD3;
global_qTD3.token.bit.ioc = 1;

/*
	Shove it in the queuehead
*/
queue_head.next_qtd_pointer = &global_qTD1;

/*
	Shove the queuehead in the async list
*/
HW_USBC_UH1_ASYNCLISTADDR.B.ASYBASE = (uint32_t)&queue_head;

debug_dump_buffer((unsigned char *)&queue_head, 0, sizeof(queue_head));

debug_print_this("HW_USBC_UH1_USBSTS : ", HW_USBC_UH1_USBSTS.U);
/*
	Enable the Async list
*/
debug_print_string("Enable Async List\r\n");
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() | BM_USBC_UH1_USBCMD_ASE  | BM_USBC_UH1_USBCMD_IAA);
while(!(HW_USBC_UH1_USBSTS_RD() & BM_USBC_UH1_USBSTS_AS))
	;	/* do nothing */


delay_us(1000*1000);

debug_dump_buffer((unsigned char *)&queue_head, 0, sizeof(queue_head));

debug_print_this("HW_USBC_UH1_USBSTS : ", HW_USBC_UH1_USBSTS.U);
HW_USBC_UH1_USBSTS.U = HW_USBC_UH1_USBSTS.U;

debug_print_this("HW_USBC_UH1_USBSTS : ", HW_USBC_UH1_USBSTS.U);
/*
	Wait for it to terminate
*/
debug_print_string("Wait for it to finish\r\n");
while(!(HW_USBC_UH1_USBSTS_RD() & BM_USBC_UH1_USBSTS_UI));
	HW_USBC_UH1_USBSTS_WR(HW_USBC_UH1_USBSTS_RD() | BM_USBC_UH1_USBSTS_UI);

/*
	Wait for the final object to effect
*/
//while (global_qTD3.token.bit.status == ATOSE_usb_ehci_queue_element_transfer_descriptor_token::STATUS_ACTIVE)
//	; /* do nothing */

/*
	Disable the async list
*/
debug_print_string("Disable Async List\r\n");
HW_USBC_UH1_USBCMD_WR(HW_USBC_UH1_USBCMD_RD() & (~BM_USBC_UH1_USBCMD_ASE));
while(HW_USBC_UH1_USBCMD_RD() & BM_USBC_UH1_USBCMD_ASE)
	;	/* nothing */
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

debug_print_string("\r\n[HW_USBC_UH1_USBSTS:");
debug_print_hex(usb_status.U);

/*
	After initialisation and under normal operation we expect only this case.
*/
if (usb_status.B.UI)
	debug_print_string("USB INTERRUPT]\r\n");

else if (usb_status.B.UI)
	debug_print_string("USB ERROR]\r\n");
/*
	USB Reset Interrupt
*/
else if (usb_status.B.URI)
	debug_print_string("USB RESET]\r\n");

/*
	USB Port Change Interrupt
	If we disable this then when the user disconnects and reconnects then we don't get a reset message!!!
*/
else if (usb_status.B.PCI)
	{
	debug_print_string("USB PCI:\r\n");

	debug_print_this("HW_USBC_UH1_PORTSC1    :", HW_USBC_UH1_PORTSC1.U);
	debug_print_this("BM_USBC_UH1_PORTSC1_CCS:", BM_USBC_UH1_PORTSC1_CCS);

	debug_print_string("Waiting for USB connected...\r\n");
	while(!(HW_USBC_UH1_PORTSC1_RD() & BM_USBC_UH1_PORTSC1_CCS))
		; /* nothing */
	debug_print_string("Connect detected.\r\n");

	usb_bus_reset();
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
	delay_us(1000 * 1000);	// wait 1 second

	ATOSE_usb_setup_data setup_packet;
	ATOSE_usb_standard_device_descriptor descriptor;

	setup_packet.bmRequestType.all = 0x80;
	setup_packet.bRequest = ATOSE_usb::REQUEST_GET_DESCRIPTOR;
	setup_packet.wValue = ATOSE_usb::DESCRIPTOR_TYPE_DEVICE;
	setup_packet.wIndex = 0;
	setup_packet.wLength = 8;

	memset(&descriptor, 0xFF, sizeof(descriptor));

	debug_print_string("About to call send_setup_packet_to_device\r\n");

	send_setup_packet_to_device(0, 0, &setup_packet, &descriptor);

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

	memset(&descriptor, 0xFF, sizeof(descriptor));

	send_setup_packet_to_device(0, 0, &setup_packet, &descriptor);

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


/*
	VOID DUMP_PORTSC1(VOID)
	-----------------------
*/
void dump_portsc1(void)
{
debug_print_this("Current Connect Status                        :", HW_USBC_UH1_PORTSC1.B.CCS);
debug_print_this("Connect Status Change                         :", HW_USBC_UH1_PORTSC1.B.CSC);
debug_print_this("Port Enabled/Disabled                         :", HW_USBC_UH1_PORTSC1.B.PE);
debug_print_this("Port Enable/Disable Change                    :", HW_USBC_UH1_PORTSC1.B.PEC);
debug_print_this("Over-current Active                           :", HW_USBC_UH1_PORTSC1.B.OCA);
debug_print_this("Over-current Change                           :", HW_USBC_UH1_PORTSC1.B.OCC);
debug_print_this("Force Port Resume                             :", HW_USBC_UH1_PORTSC1.B.FPR);
debug_print_this("Suspend                                       :", HW_USBC_UH1_PORTSC1.B.SUSP);
debug_print_this("Port Reset                                    :", HW_USBC_UH1_PORTSC1.B.PR);
debug_print_this("High-Speed Port                               :", HW_USBC_UH1_PORTSC1.B.HSP);
debug_print_this("Line Status                                   :", HW_USBC_UH1_PORTSC1.B.LS);
debug_print_this("Port Power (PP)                               :", HW_USBC_UH1_PORTSC1.B.PP);
debug_print_this("Port Owner                                    :", HW_USBC_UH1_PORTSC1.B.PO);
debug_print_this("Port Indicator Control                        :", HW_USBC_UH1_PORTSC1.B.PIC);
debug_print_this("Port Test Control                             :", HW_USBC_UH1_PORTSC1.B.PTC);
debug_print_this("Wake on Connect Enable (WKCNNT_E)             :", HW_USBC_UH1_PORTSC1.B.WKCN);
debug_print_this("Wake on Disconnect Enable (WKDSCNNT_E)        :", HW_USBC_UH1_PORTSC1.B.WKDC);
debug_print_this("Wake on Over-current Enable (WKOC_E)          :", HW_USBC_UH1_PORTSC1.B.WKOC);
debug_print_this("PHY Low Power Suspend - Clock Disable (PLPSCD):", HW_USBC_UH1_PORTSC1.B.PHCD);
debug_print_this("Port Force Full Speed Connect                 :", HW_USBC_UH1_PORTSC1.B.PFSC);
debug_print_this("See description at bits 31-30                 :", HW_USBC_UH1_PORTSC1.B.PTS_2);
debug_print_this("Port Speed                                    :", HW_USBC_UH1_PORTSC1.B.PSPD);
debug_print_this("Parallel Transceiver Width                    :", HW_USBC_UH1_PORTSC1.B.PTW);
debug_print_this("Serial Transceiver Select                     :", HW_USBC_UH1_PORTSC1.B.STS);
debug_print_this("Bit field {bit25, bit31, bit30}               :", HW_USBC_UH1_PORTSC1.B.PTS_1);
}

/*
	DO_SOME_MAGIC(VOID)
	-------------------
*/
void do_some_magic(void)
{
debug_print_string("\r\nWaiting for USB connected...\r\n");
while(!(HW_USBC_UH1_PORTSC1_RD() & BM_USBC_UH1_PORTSC1_CCS))
	; /* nothing */
debug_print_string("Connect detected.\r\n");

dump_portsc1();

debug_print_string("Bus Reset.\r\n");
	usb_bus_reset();
delay_us(1000*1000);
debug_print_string("Bus Reset.\r\n");
	usb_bus_reset();
delay_us(1000*1000);

dump_portsc1();


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

memset(&descriptor, 0xFF, sizeof(descriptor));

debug_print_string("About to call send_setup_packet_to_device\r\n");

send_setup_packet_to_device(0, 0, &setup_packet, &descriptor);

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

memset(&descriptor, 0xFF, sizeof(descriptor));

send_setup_packet_to_device(0, 0, &setup_packet, &descriptor);

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
