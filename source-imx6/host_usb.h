/*
	HOST_USB.H
	----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_H_
#define HOST_USB_H_

#include "device_driver.h"

class ATOSE_usb_ehci_queue_head;
class ATOSE_usb_ehci_queue_element_transfer_descriptor;
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
	ATOSE_semaphore *semaphore;
	uint32_t semaphore_handle;

protected:
	void usb_bus_reset(void);
	void initialise_queuehead(ATOSE_usb_ehci_queue_head *queue_head, uint32_t device, uint32_t endpoint);
	void initialise_transfer_descriptor(ATOSE_usb_ehci_queue_element_transfer_descriptor *descriptor, uint32_t transaction_type, char *data, uint32_t data_length);

	void send_setup_packet_to_device(uint32_t device, uint32_t endpoint, ATOSE_usb_setup_data *packet, void *descriptor, uint8_t size);
	void read_from_interrupt_port(uint32_t device, uint32_t endpoint, void *buffer, uint8_t size);

	void wait_for_connection(void);
	ATOSE_usb_standard_device_descriptor *get_device_descriptor(ATOSE_usb_standard_device_descriptor *descriptor);
	ATOSE_usb_standard_configuration_descriptor *get_configuration_descriptor(uint32_t address, ATOSE_usb_standard_configuration_descriptor *descriptor, uint32_t size = 0);
	void set_address(uint32_t address);
	void set_configuration(uint32_t address, uint32_t configuration);
	void get_hub_status(uint32_t address);
	void hub_connect_status(uint32_t address, uint32_t endpoint, uint8_t *buffer, uint32_t bytes);
	void hub_get_best_configuration(ATOSE_usb_standard_configuration_descriptor *configuration_descriptor, uint32_t *best_interface, uint32_t *best_alternate, uint32_t *best_endpoint);
	void hub_set_interface(uint32_t address, uint32_t interface, uint32_t alternate);
	void get_hub_descriptor(uint32_t address, ATOSE_usb_standard_hub_descriptor *descriptor, uint32_t size = 0);

	void get_port_status(uint32_t address, uint32_t port, uint8_t *buffer);
	void set_port_feature(uint32_t address, uint32_t port, uint32_t feature);

public:
	ATOSE_host_usb();

	virtual void enable(void);
	virtual void disable(void) {}
	virtual void acknowledge(ATOSE_registers *registers);
	virtual uint32_t get_interrup_id(void);

	void device_manager(void);
} ;

#endif /* HOST_USB_H_ */
