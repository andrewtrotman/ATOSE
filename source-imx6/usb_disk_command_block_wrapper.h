/*
	USB_DISK_COMMAND_BLOCK_WRAPPER.H
	--------------------------------
*/
#ifndef USB_DISK_COMMAND_BLOCK_WRAPPER_H_
#define USB_DISK_COMMAND_BLOCK_WRAPPER_H_

/*
	ATOSE_USB_DISK_COMMAND_BLOCK_WRAPPER
	------------------------------------
*/
class ATOSE_usb_disk_command_block_wrapper
{
public:
	static const uint32_t SIGNATURE = 0x43425355;
	static const uint8_t  FLAG_DIRECTION_OUT = 0x00;
	static const uint8_t  FLAG_DIRECTION_IN  = 0x80;

public:
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t  bmCBWFlags;
	uint8_t  bCBWLUN;
	uint8_t  bCBWCBLength;
	uint8_t  CBWCB[16];
};

#endif
