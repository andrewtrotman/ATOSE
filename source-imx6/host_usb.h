/*
	HOST_USB.H
	----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_H_
#define HOST_USB_H_

#include "device_driver.h"
#include "host_usb_device.h"
#include "usb_ehci_queue_element_transfer_descriptor.h"
#include "usb_ehci_queue_head.h"

class ATOSE_usb_setup_data;
class ATOSE_usb_standard_device_descriptor;
class ATOSE_usb_standard_configuration_descriptor;
class ATOSE_usb_standard_hub_descriptor;
class ATOSE_semaphore;

/*
	class ATOSE_HOST_USB
	--------------------
*/
class ATOSE_host_usb : public ATOSE_device_driver
{
private:
	static const uint32_t MAX_USB_DEVICES = 128;

private:
	/*
	   The i.MX6Q SDK (iMX6_Platform_SDK\sdk\drivers\usb\src\usbh_drv.c) alligns on 64-byte
		boundaries (see usbh_qh_init()).  We stick with 64 because that seems to work well.
	*/
	ATOSE_usb_ehci_queue_head queue_head __attribute__ ((aligned(64)));
	ATOSE_usb_ehci_queue_head empty_queue_head __attribute__ ((aligned(64)));

	/*
		Although the reference manual allignes transfer descriptors on 32-byte boundaries, the
		i.MX6Q SDK (iMX6_Platform_SDK\sdk\drivers\usb\src\usbh_drv.c) alligns them on 64-byte
		boundaries (see usbh_qtd_init()).  We'll stick with 64 because it seems to work well.
	*/
	ATOSE_usb_ehci_queue_element_transfer_descriptor transfer_descriptor_1 __attribute__ ((aligned(64)));
	ATOSE_usb_ehci_queue_element_transfer_descriptor transfer_descriptor_2 __attribute__ ((aligned(64)));
	ATOSE_usb_ehci_queue_element_transfer_descriptor transfer_descriptor_3 __attribute__ ((aligned(64)));

	/*
		We need the semaphore to communicate between the system process and the IRQ
	*/
	ATOSE_semaphore *semaphore;
	uint32_t semaphore_handle;

	/*
		As this code represents a port (essentially a USB Bus), we have some set of attached devices
	*/
	ATOSE_host_usb_device device_list[MAX_USB_DEVICES];
	uint32_t device_list_length;

protected:
	void initialise_queuehead(ATOSE_usb_ehci_queue_head *queue_head, ATOSE_host_usb_device *device, uint32_t endpoint);
	void initialise_transfer_descriptor(ATOSE_usb_ehci_queue_element_transfer_descriptor *descriptor, uint32_t transaction_type, char *data, uint32_t data_length);
	uint32_t wait_for_connection(void);
	uint32_t usb_bus_reset(void);
	ATOSE_host_usb_device *enumerate(uint8_t parent, uint8_t transaction_translator_address, uint8_t transaction_translator_port, uint8_t my_velocity);

	/*
		Experimental methods
	*/
	void hub_connect_status(ATOSE_host_usb_device *device, uint32_t endpoint, uint8_t *buffer, uint32_t bytes);
	void hub_get_best_configuration(ATOSE_usb_standard_configuration_descriptor *configuration_descriptor, uint32_t *best_interface, uint32_t *best_alternate, uint32_t *best_endpoint);

public:
	ATOSE_host_usb();

	/*
		Device driver methods
	*/
	virtual void enable(void);
	virtual void disable(void) {}
	virtual void acknowledge(ATOSE_registers *registers);
	virtual uint32_t get_interrup_id(void);

	/*
		This object's behaviours
	*/
	void device_manager(void);
	uint32_t send_setup_packet(ATOSE_host_usb_device *device, uint32_t endpoint, ATOSE_usb_setup_data *packet, void *descriptor, uint8_t size);
	uint32_t read_interrupt_packet(ATOSE_host_usb_device *device, uint32_t endpoint, void *buffer, uint8_t size);
	uint32_t send_and_recieve_packet(ATOSE_host_usb_device *device, uint32_t endpoint, void *packet, uint8_t packet_size, void *descriptor, uint8_t size);
} ;

#endif /* HOST_USB_H_ */
