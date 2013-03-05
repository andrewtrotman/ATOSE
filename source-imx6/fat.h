/*
	FAT.H
	-----
*/
#ifndef FAT_H_
#define FAT_H_

#include <stdint.h>
#include "host_usb_device_disk.h"

/*
	class ATOSE_FAT
	---------------
*/
class ATOSE_fat
{
protected:
	uint64_t base;										// the offset of the FAT-32 volume from the from the start of the physical device
	ATOSE_host_usb_device_disk *disk;
	uint32_t read_sector(void *buffer, uint64_t sector) { return disk->read_sector(buffer, sector + base); }

	uint32_t dead_volume;								// true if we can't read this volume and should consider it dead
	uint64_t clusters_on_disk;
	uint64_t root_directory_location;

public:
	ATOSE_fat(ATOSE_host_usb_device_disk *disk, uint64_t base = 0);
	void dir(void);
} ;

#endif
