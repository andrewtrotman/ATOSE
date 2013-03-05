/*
	USB_DISK_COMMAND_STATUS_WRAPPER.H
	---------------------------------
*/
#ifndef USB_DISK_COMMAND_STATUS_WRAPPER_H_
#define USB_DISK_COMMAND_STATUS_WRAPPER_H_

/*
	class ATOSE_USB_DISK_COMMAND_STATUS_WRAPPER
*/
class ATOSE_usb_disk_command_status_wrapper
{
public:
	static const uint32_t SIGNATURE = 0x53425355;

	static const uint8_t STATUS_PASED = 0x00;
	static const uint8_t STATUS_FAILED = 0x01;
	static const uint8_t STATUS_PHASE_ERROR = 0x02;

public:
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t  bCSWStatus;
} __attribute__ ((packed));

#endif
