/*
	HOST_USB_DEVICE_HUB.C
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "usb_hub.h"
#include "host_usb_device_hub.h"
#include "usb_hub_port_status_and_change.h"
#include "host_usb.h"
#include "usb_standard_configuration_descriptor.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"


/*
	ATOSE_HOST_USB_DEVICE_HUB::ATOSE_HOST_USB_DEVICE_HUB()
	------------------------------------------------------
*/
ATOSE_host_usb_device_hub::ATOSE_host_usb_device_hub(ATOSE_host_usb_device *details) : ATOSE_host_usb_device(details)
{
uint8_t child_velocity;
uint32_t port;
ATOSE_usb_standard_hub_descriptor hub_descriptor;
ATOSE_usb_hub_port_status_and_change status;

if (get_hub_descriptor(&hub_descriptor) != 0)
	return;

hub_ports = hub_descriptor.bNbrPorts;

for (port = 1; port <= hub_ports; port++)
	{
	/*
		Power up the port
	*/
	if (set_port_feature(port, ATOSE_usb_hub::PORT_POWER) == 0)
		{
		if (get_port_status(port, &status) == 0)
			{
			if (status.status.port_connection)
				{
				/*
					Reset the port
				*/
				set_port_feature(port, ATOSE_usb_hub::PORT_RESET);
				clear_port_feature(port, ATOSE_usb_hub::C_PORT_CONNECTION);
				clear_port_feature(port, ATOSE_usb_hub::C_PORT_RESET);

				/*
					At this point (as bizzar as this is gonna sound), devices attached to the port can choose to change their speeds.
					I have observed this happening, and we can't just ignore it because otherwise we end up using a transaction translator
					when we shouldn't and it all goes to pot.
				*/
				if (get_port_status(port, &status) == 0)
					{
					/*
						Do that recursive thing (enumerate my children)
						If the child is USB 1.1 and I'm USB 2.0 then I'm the translator, else my translator is my child's translator
					*/
					child_velocity = status.status.port_low_speed ? ATOSE_host_usb_device::VELOCITY_LOW : status.status.port_high_speed ? ATOSE_host_usb_device::VELOCITY_HIGH : ATOSE_host_usb_device::VELOCITY_FULL;
					if (port_velocity == ATOSE_host_usb_device::VELOCITY_HIGH && (child_velocity == ATOSE_host_usb_device::VELOCITY_LOW || child_velocity == ATOSE_host_usb_device::VELOCITY_FULL))
						ehci->enumerate(address, address, port, child_velocity);
					else
						ehci->enumerate(address, transaction_translator_address, transaction_translator_port, child_velocity);
					}
				}
			}
		}
	}
dead = false;
}



/*
	ATOSE_HOST_USB_DEVICE_HUB::HUB_GET_BEST_CONFIGURATION()
	-------------------------------------------------------
*/
void ATOSE_host_usb_device_hub::hub_get_best_configuration(ATOSE_usb_standard_configuration_descriptor *configuration_descriptor, uint32_t *best_interface, uint32_t *best_alternate, uint32_t *best_endpoint)
{
ATOSE_usb_standard_endpoint_descriptor *endpoint_descriptor;
ATOSE_usb_standard_interface_descriptor *interface_descriptor;
uint8_t *descriptor, *end;
uint32_t best_protocol;
uint32_t current_protocol, current_interface, current_alternate, current_endpoint;

best_protocol = 0;
*best_interface = *best_alternate = current_protocol = current_interface = current_alternate = 0;
*best_endpoint = current_endpoint = 1;

descriptor = (uint8_t *)configuration_descriptor;
end = descriptor + configuration_descriptor->wTotalLength;
while (descriptor < end)
	{
	interface_descriptor = (ATOSE_usb_standard_interface_descriptor *)descriptor;
	descriptor += interface_descriptor->bLength;
	if (interface_descriptor->bDescriptorType == ATOSE_usb::DESCRIPTOR_TYPE_INTERFACE)
		{
		current_interface = interface_descriptor->bInterfaceNumber;
		current_alternate = interface_descriptor->bAlternateSetting;
		current_protocol = interface_descriptor->bInterfaceProtocol;
		}
	if (interface_descriptor->bDescriptorType == ATOSE_usb::DESCRIPTOR_TYPE_ENDPOINT)
		{
		endpoint_descriptor = (ATOSE_usb_standard_endpoint_descriptor *)interface_descriptor;
		current_endpoint = endpoint_descriptor->bEndpointAddress & 0x7F;

		if (current_protocol >= best_protocol)
			{
			best_protocol = current_protocol;
			*best_interface = current_interface;
			*best_alternate = current_alternate;
			*best_endpoint = current_endpoint;
			}
		}
	}
}

