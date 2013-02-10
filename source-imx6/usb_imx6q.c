/*
	USB_IMX6Q.C
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "ascii_str.h"
#include "usb_imx6q.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbcore.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbphy.h"


/*
	Pointers to memory (use for allocators)
*/
extern uint32_t ATOSE_start_of_heap;
char *external_memory_head = (char *)&ATOSE_start_of_heap;

/*
	MAIN_MEMORY_ALLOC()
	-------------------
	Allocate "size" bytes of memory alliged on an "alignment" boundey
*/
void* main_memory_alloc(size_t size, unsigned int alignment)
{
void *result;

/*
	Align the beginning to the requested alignment
*/
uint32_t realignment = alignment - ((uint32_t) external_memory_head % alignment);

if (realignment > 0 && realignment < alignment)
	external_memory_head += realignment;

/*
	Now return the current pointer and move on
*/
result = external_memory_head;
external_memory_head += size;

return result;
}




/*
	ATOSE_USB_IMX6Q::ATOSE_USB_IMX6Q()
	----------------------------------
	Do all the magic to turn on the USB.  Recall that we are booting over USB so clock gating is already done
*/
ATOSE_usb_imx6q::ATOSE_usb_imx6q() : ATOSE_usb()
{
/*
	Tell the controller we're a device
*/
HW_USBC_UOG_USBMODE.B.CM = 2;			// DEVICE MODE

/*
	Clear all the status bits
*/
HW_USBC_UOG_ENDPTSETUPSTAT.U = HW_USBC_UOG_ENDPTSETUPSTAT.U;
HW_USBC_UOG_ENDPTCOMPLETE.U = HW_USBC_UOG_ENDPTCOMPLETE.U;
HW_USBC_UOG_ENDPTSTAT.U = HW_USBC_UOG_ENDPTSTAT.U;

/*
	We want to know about the following events:
		BM_USBC_UOG_USBSTS_URI |	// USB USB Reset Received
		BM_USBC_UOG_USBSTS_PCI |	// USB Port Change Detect
		BM_USBC_UOG_USBSTS_UI		// USB Interrupt (USBINT)
*/
HW_USBC_UOG_USBSTS_SET(BM_USBC_UOG_USBSTS_URI | BM_USBC_UOG_USBSTS_PCI | BM_USBC_UOG_USBSTS_UI);

/*
	Enable interrupts
	The Port Change Detect interrupt is essential for a disconnect and reconnect while we're powered up.  If
	we don't enable the port connect then we don't get the reset interrupt to tell us we've just connected to
	a host.
*/
HW_USBC_UOG_USBINTR_WR(BM_USBC_UOG_USBINTR_URE | BM_USBC_UOG_USBINTR_PCE | BM_USBC_UOG_USBINTR_UE);

/*
	Set the port speed, options are:
		BV_USBC_UOG_PORTSC1_PSPD__FULL  0		// 12Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__LOW   1		// 1.5Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__HIGH  2		// 480Mb/s
*/
HW_USBC_UOG_PORTSC1.B.PSPD = 2;

/*
	Turn on the trip-wire mechanism for Setup packets
*/
HW_USBC_UOG_USBMODE.B.SLOM = 1;

/*
	now start the USB sub-system
*/
HW_USBC_UOG_USBCMD.B.RS = 1;
}

/*
	ATOSE_USB_IMX6Q::SEND_TO_HOST()
	-------------------------------
	Queue up a transfer IN from the device to the host.  When the transfer has completed and interrupt
	will be issued.  This method converts from endpoint numbers to queue head numbers as queue-heads
	appear to be Freescale specific.
*/
void ATOSE_usb_imx6q::send_to_host(uint32_t endpoint, const uint8_t *buffer, uint32_t length)
{
uint32_t mask;
ATOSE_usb_imx6q_endpoint_transfer_descriptor *descriptor;
ATOSE_usb_imx6q_endpoint_queuehead *queuehead;

/*
	Set up the transfer descriptor
*/
descriptor = &port_transfer_descriptor[endpoint * 2 + 1];
memset(descriptor, 0, sizeof(*descriptor));
descriptor->next_link_pointer = (ATOSE_usb_imx6q_endpoint_transfer_descriptor *)ATOSE_usb_imx6q_endpoint_transfer_descriptor::TERMINATOR;
descriptor->token.all = 0;
descriptor->token.bit.total_bytes = length;
descriptor->token.bit.ioc = 1;
descriptor->token.bit.status = ATOSE_usb_imx6q_endpoint_transfer_descriptor::STATUS_ACTIVE;
descriptor->buffer_pointer[0] = (char *)buffer;

/*
	Set up the queue head
*/
queuehead = &port_queuehead[endpoint * 2 + 1];
queuehead->dtd_overlay_area.next_link_pointer = descriptor;

/*
	Prime the i.MX USB hardware
*/
mask = BF_USBC_UOG_ENDPTPRIME_PETB(1 << endpoint);
HW_USBC_UOG_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBC_UOG_ENDPTPRIME_RD() & mask) != 0)
	; /* do nothing */
}

/*
	ATOSE_USB_IMX6Q::RECIEVE_FROM_HOST()
	------------------------------------
	Queue up a transfer OUT from the host to the device.  When the message has arrived from the host we
	get an interrupt. This method converts from endpoint numbers to queue head numbers as queueheads
	appear to be Freescale specific
*/
void ATOSE_usb_imx6q::recieve_from_host(uint32_t endpoint)
{
uint32_t mask = 0;
ATOSE_usb_imx6q_endpoint_transfer_descriptor *descriptor;
ATOSE_usb_imx6q_endpoint_queuehead *queuehead;

/*
	Set up the transfer descriptor

	As we can't know how big the transfer is going to be we'll use the same size we allocate
	for the transfer buffer (below).  If the packet is short that'll be OK as the protocol allows
	for short packets. If it is longer the hardware will NAK (I think)
*/
descriptor =  &port_transfer_descriptor[endpoint * 2];
memset(descriptor, 0, sizeof(*descriptor));
descriptor->next_link_pointer = (ATOSE_usb_imx6q_endpoint_transfer_descriptor *)ATOSE_usb_imx6q_endpoint_transfer_descriptor::TERMINATOR;;
descriptor->token.all = 0;
descriptor->token.bit.total_bytes = QUEUE_BUFFER_SIZE;
descriptor->token.bit.ioc = 1;
descriptor->token.bit.status = ATOSE_usb_imx6q_endpoint_transfer_descriptor::STATUS_ACTIVE;
descriptor->buffer_pointer[0] = (char *)global_transfer_buffer[endpoint * 2][0];		// the recieve buffer

/*
	Set up the queue head
*/
queuehead = &port_queuehead[endpoint * 2];
queuehead->dtd_overlay_area.next_link_pointer = descriptor;

/*
	Prime the i.MX USB hardware
*/
mask = BF_USBC_UOG_ENDPTPRIME_PERB(1 << endpoint);
HW_USBC_UOG_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBC_UOG_ENDPTPRIME_RD() & mask) != 0)
	; /* do nothing */
}

/*
	ATOSE_USB_IMX6Q::SIGNAL_AN_ERROR()
	----------------------------------
	If the host issues a command and we are unable to respond to it we are supposed to return a STALL.  I doubt this will
	ever happen, but just in case it does.
	See page 247 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
*/
void ATOSE_usb_imx6q::signal_an_error(uint32_t endpoint)
{
#define HW_USBC_UOG_ENDPTCTRLn_ADDR(n)      (REGS_USBC_BASE + 0x000001C0 + ((n) * 0x4))
#define HW_USBC_UOG_ENDPTCTRLn(n)           (*(volatile hw_usbc_uog_endptctrl0_t *) HW_USBC_UOG_ENDPTCTRLn_ADDR(n))
#define HW_USBC_UOG_ENDPTCTRLn_RD(n)        (HW_USBC_UOG_ENDPTCTRLn(n).U)
#define HW_USBC_UOG_ENDPTCTRLn_WR(n, v)     (HW_USBC_UOG_ENDPTCTRLn(n).U = (v))

HW_USBC_UOG_ENDPTCTRLn_WR(endpoint, HW_USBC_UOG_ENDPTCTRLn_RD(endpoint) & BM_USBC_UOG_ENDPTCTRL0_TXS);
}

/*
	ATOSE_USB_IMX6Q::ENABLE()
	-------------------------
*/
void ATOSE_usb_imx6q::enable(void)
{
/*
	Disable the Phy.  This will result in the host dropping the connection
	which is necessary because we want to change who we are (from the i.MX6Q
	in boot mode to ATOSEE).
*/
HW_USBPHY_CTRL(1).B.CLKGATE = 1;

/*
	Wait long enough that the host notices what's happened
*/
ATOSE_atose::get_ATOSE()->cpu.delay_us(1000000);			// 1 second delay

/*
	Now reconnect
*/
HW_USBPHY_CTRL(1).B.CLKGATE = 0;
while (HW_USBPHY_CTRL(1).B.CLKGATE)
	; /* do nothing */
}

/*
	ATOSE_USB_IMX6Q::ACKNOWLEDGE()
	------------------------------
*/
void ATOSE_usb_imx6q::acknowledge(void)
{
hw_usbc_uog_usbsts_t usb_status;

/*
	Grab the cause of the interrupt and acknowledge it
*/
usb_status.U = HW_USBC_UOG_USBSTS.U;
HW_USBC_UOG_USBSTS.U = usb_status.U;

/*
	After initialisation and under normal operation we expect only this case.
*/
if (usb_status.B.UI)
	usb_interrupt();

/*
	USB Reset Interrupt
*/
if (usb_status.B.URI)
	reset_interrupt();

/*
	USB Port Change Interrupt
	If we disable this then when the user disconnects and reconnects then we don't get a reset message!!!
*/
if (usb_status.B.PCI)
	portchange_interrupt();
}

/*
	ATOSE_USB_IMX6Q::PORTCHANGE_INTERRUPT()
	---------------------------------------
*/
void ATOSE_usb_imx6q::portchange_interrupt(void)
{
/*
	Page 5319 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	We get this interrupt when the hardware detects:
		Connect Status Change
		Port Enable/Disable Change
		Over-current Change
		Force Port Resume
*/
}

/*
	ATOSE_USB_IMX6Q::RESET_INTERRUPT()
	----------------------------------
*/
void ATOSE_usb_imx6q::reset_interrupt(void)
{
/*
	This happens when the host wants to re-initialise the USB bus
	which also happens on startup.

	See page 5338-5339 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	where it states that on this interrupt we must:

	"Clear all setup token semaphores by reading the Endpoint Status
	 (USBC_n_ENDPTSTAT) register and writing the same value back to the Endpoint
	 Status (USBC_n_ENDPTSTAT) register.

	 Clear all the endpoint complete status bits by reading the Endpoint Complete
	 (USBC_n_ENDPTCOMPLETE) register and writing the same value back to the
	 Endpoint Complete (USBC_n_ENDPTCOMPLETE) register.

	 Cancel all primed status by waiting until all bits in the Endpoint Prime
	 (USBC_n_ENDPTPRIME) are 0 and then writing 0xFFFFFFFF to Endpoint Flush
	 (USBC_n_ENDPTFLUSH).

	 Read the reset bit in the Port Status & Control (USBC_n_PORTSC1) register and make
	 sure that it is still active. A USB reset will occur for a minimum of 3 ms and the DCD
	 must reach this point in the reset cleanup before end of the reset occurs, otherwise a
	 hardware reset of the device controller is recommended (rare.)

		A hardware reset can be performed by writing a one to the device controller reset bit
		in the USBCMD reset. Note: a hardware reset will cause the device to detach from
		the bus by clearing the Run/Stop bit. Thus, the DCD must completely re-initialize the
		device controller after a hardware reset.

	 Free all allocated dTDs because they will no longer be executed by the device controller.
	 If this is the first time the DCD is processing a USB reset event, then it is likely that no
	 dTDs have been allocated.

	 At this time, the DCD may release control back to the OS because no further changes to
	 the device controller are permitted until a Port Change Detect is indicated.
	 After a Port Change Detect, the device has reached the default state and the DCD can
	 read the Port Status & Control (USBC_n_PORTSC1) to determine if the device is
	 operating in FS or HS mode. At this time, the device controller has reached normal
	 operating mode and DCD can begin enumeration according to the USB Chapter 9 -
	 Device Framework."

*/

HW_USBC_UOG_ENDPTSTAT.U = HW_USBC_UOG_ENDPTSTAT_RD();			// first point
HW_USBC_UOG_ENDPTCOMPLETE_WR(HW_USBC_UOG_ENDPTCOMPLETE_RD());	// second point

while (HW_USBC_UOG_ENDPTPRIME_RD() != 0)						// third point
	 /* do nothing */ ;
HW_USBC_UOG_ENDPTFLUSH_WR(0xFFFFFFFF);							// third point

if (HW_USBC_UOG_PORTSC1.B.PR != 1)								// fourth point
	{
	/*
		Hard reset needed (which we ignore at the moment)
	*/
	}
/*
	We can ignore the fifth point because they are already allocated.
	We ignore the sixth point as this is normal behaviour
	We do not need to change our address back to 0 because that automatically done for us
*/
}

/*
	ATOSE_USB_IMX6Q::USB_INTERRUPT()
	--------------------------------
*/
void ATOSE_usb_imx6q::usb_interrupt(void)
{
uint32_t endpoint;
uint32_t endpoint_setup_status;
uint32_t endpoint_status;
uint32_t endpoint_complete;
ATOSE_usb_setup_data setup_packet;

/*
	Grab the value of the various status registers as their values can change while this routine is running
*/
endpoint_setup_status = HW_USBC_UOG_ENDPTSETUPSTAT_RD();
endpoint_status = HW_USBC_UOG_ENDPTSTAT_RD();
endpoint_complete = HW_USBC_UOG_ENDPTCOMPLETE_RD();

/*
	Tell the i.MX that we've serviced those requests as quickly as we can so that we don't lose any messages
*/
HW_USBC_UOG_ENDPTSETUPSTAT_WR(endpoint_setup_status);
HW_USBC_UOG_ENDPTCOMPLETE_WR(endpoint_complete);
HW_USBC_UOG_ENDPTSTAT.U = endpoint_status;

/*
	The i.MX series has special handeling of a setup message - it puts it directly into the
	transfer descriptor rather than the transfer buffer.  It also sets an internal flag saying
	which endpoint just got the setup message and then generates an interupt.  We check for
	that flag here.
*/
if (endpoint_setup_status & 0x1F)
	{
	/*
		In the CDC model (and presumably elsewhere) setup packets can only occur on Endpoint 1. If we
		get a setup packet anywhere else then its an error.
	*/
	if (endpoint_setup_status & 0x01)
		{
		/*
			The way we handle a setup event is explicity stated on page 5347-5348 of "i.MX 6Dual/6Quad
			Applications Processor Reference Manual Rev. 0, 11/2012"  where it states:

			"* Disable Setup Lockout by writing 1 to Setup Lockout Mode (SLOM) in USB Device
			   Mode (USBC_n_USBMODE). (once at initialization). Setup lockout is not necessary
			   when using the tripwire as described below."

			That is done when we enable the USB sub-system, but here we must:


			"* After receiving an interrupt and inspecting Endpoint Setup Status
			   (USBC_n_ENDPTSETUPSTAT) to determine that a setup packet was received on a
			   particular pipe:
			   a. Write 1 to clear corresponding bit Endpoint Setup Status
			      (USBC_n_ENDPTSETUPSTAT).
			   b. Write 1 to Setup Tripwire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register.
			   c. Duplicate contents of dQH.SetupBuffer into local software byte array.
			   d. Read Setup TripWire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register. (if set - continue; if cleared - goto 2)
			   e. Write 0 to clear Setup Tripwire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register.
			   f. Process setup packet using local software byte array copy and execute status/
			      handshake phases."

			OK, so point "d" should read "goto b" not "goto 2" (this is also wrong in the i.MX53 manual).  But
			otherwise, this is what we do here.
		*/

		HW_USBC_UOG_ENDPTSETUPSTAT_WR(1);										// bullet-point "a": clear status register
		do
			{
			HW_USBC_UOG_USBCMD.B.SUTW = 1;										// bullet-point "b": set tripwire
			setup_packet = port_queuehead[0].setup_buffer;							// bullet-point "c": copy packet
			}
		while (HW_USBC_UOG_USBCMD.B.SUTW == 0);									// bullet-point "d": check tripwire
		HW_USBC_UOG_USBCMD.B.SUTW = 0;											// bullet-point "e": clear tripwire

		/*
			bullet-point "f": process the packet
		*/
		process_setup_packet(&setup_packet);
		}
	}

/*
	Manage any send/recieve completion events and any notifications that the endpoints are no longer primed
*/
for (endpoint = 0; endpoint < MAX_ENDPOINTS; endpoint++)
	{
	/*
		Recieve on and endpoint has completed
	*/
	if (endpoint_complete & BF_USBC_UOG_ENDPTCOMPLETE_ERCE(1 << endpoint))
		if (endpoint != 0)
			{
			/*
				If we're on the data endpoint then echo back and queue up a request to recieve data
			*/
			send_to_host(endpoint, (uint8_t *)&((global_transfer_buffer[endpoint * 2][0])[0]), 1);
			recieve_from_host(endpoint);
			}

	/*
		If we're on endpoint 0 then we make sure we are always ready to receive
	*/
#define BF_USBC_UOG_ENDPTSTAT_ERBR(v)   (((v) << 0) & BM_USBC_UOG_ENDPTSTAT_ERBR)

	if (endpoint == 0 && (!(endpoint_status & BF_USBC_UOG_ENDPTSTAT_ERBR(1 << endpoint))))
		recieve_from_host(endpoint);

	/*
		Transmit on an endpoint has completed
	*/
	if (endpoint_complete & BF_USBC_UOG_ENDPTCOMPLETE_ETCE(1 << endpoint))
		/* nothing */ ;

	/*
		We are ready to transmit on the endpoint (i.e. queued up)
	*/
#define BF_USBC_UOG_ENDPTSTAT_ETBR(v)   (((v) << 0) & BM_USBC_UOG_ENDPTSTAT_ETBR)
	if (endpoint_status & BF_USBC_UOG_ENDPTSTAT_ETBR(1 << endpoint))
		/* nothing */ ;
	}
}

/*
	ATOSE_USB_IMX6Q::CONFIGURE_QUEUEHEAD()
	--------------------------------------
*/
void ATOSE_usb_imx6q::configure_queuehead(long which)
{
ATOSE_usb_imx6q_endpoint_transfer_descriptor *descriptor;

/*
	Get the correct transfer descriptor
*/
descriptor = &port_transfer_descriptor[which];

/*
	Terminate the linked list
*/
descriptor->next_link_pointer = (ATOSE_usb_imx6q_endpoint_transfer_descriptor *)ATOSE_usb_imx6q_endpoint_transfer_descriptor::TERMINATOR;

/*
	Set up the queuehead
*/
port_queuehead[which].dtd_overlay_area.next_link_pointer = (ATOSE_usb_imx6q_endpoint_transfer_descriptor *)(((uint32_t)descriptor) + 1);			// end of chain
port_queuehead[which].dtd_overlay_area.token.all = 0;
port_queuehead[which].dtd_overlay_area.token.bit.ioc = 1;
port_queuehead[which].capabilities.all = 0;
port_queuehead[which].capabilities.bit.maximum_packet_length = ATOSE_usb::MAX_PACKET_SIZE;
port_queuehead[which].capabilities.bit.ios = 1;

/*
	Allocate a transfer buffer and put that in the queuehead too
*/
global_transfer_buffer[which][0] = (uint8_t *)main_memory_alloc(QUEUE_BUFFER_SIZE, QUEUE_BUFFER_SIZE);
port_queuehead[which].dtd_overlay_area.buffer_pointer[0] = (void *)global_transfer_buffer[which][0];
}

/*
	ATOSE_USB_IMX6Q::ENABLE_ENDPOINT_ZERO()
	---------------------------------------
*/
void ATOSE_usb_imx6q::enable_endpoint_zero(void)
{
/*
	Zero everything (queheads then transder descriptors)
*/
memset(port_queuehead, 0, sizeof (port_queuehead));
memset(port_transfer_descriptor, 0, sizeof (port_transfer_descriptor));

/*
	Queue 0 and 1 (Endpoint 0) are necessarily for USB Control
*/
configure_queuehead(0);
configure_queuehead(1);

/*
	Hand off to the i.MX6Q
*/
HW_USBC_UOG_ENDPTLISTADDR_WR((uint32_t)port_queuehead);

/*
	Start listening
*/
recieve_from_host(0);
}

/*
	ATOSE_USB_IMX6Q::ENABLE_ENDPOINT()
	----------------------------------
*/
void ATOSE_usb_imx6q::enable_endpoint(long endpoint, long mode)
{
static uint8_t imx6q_mode[] = {0, 1, 2, 3};		// translate from ATOSE to i.MX6 modes {CONTROL, ISOCHRONOUS, BULK, INTERRUPT}
hw_usbc_uog_endptctrl1_t endpointcfg;

endpointcfg.U = 0;										// Initialise
endpointcfg.B.TXE = 1; 								// Transmit enable
endpointcfg.B.TXR = 1; 								// Reset the PID
endpointcfg.B.TXT = imx6q_mode[mode];					// Whatever mode we need
endpointcfg.B.RXE = 1;									// Recieve enable
endpointcfg.B.RXR = 1;									// Reset the PID
endpointcfg.B.RXT = imx6q_mode[mode];					// Whatever mode we need

/*
	Allocate transfer buffers and enable the endpoint
*/
configure_queuehead(endpoint * 2);
configure_queuehead(endpoint * 2 + 1);
HW_USBC_UOG_ENDPTCTRLn_WR(endpoint, endpointcfg.U);

/*
	Prime the endpoints
*/
recieve_from_host(endpoint);
}

/*
	ATOSE_USB_IMX6Q::SET_ADDRESS()
	------------------------------
*/
void ATOSE_usb_imx6q::set_address(long address)
{
HW_USBC_UOG_DEVICEADDR_WR(BF_USBC_UOG_DEVICEADDR_USBADR(address) | BM_USBC_UOG_DEVICEADDR_USBADRA);
}