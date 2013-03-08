/*
	FAT.H
	-----
*/
#ifndef FAT_H_
#define FAT_H_

#include <stdint.h>
#include "host_usb_device_disk.h"
#include "file_system.h"
#include "file_control_block.h"

class ATOSE_fat_directory_entry;

/*
	class ATOSE_FAT
	---------------
*/
class ATOSE_fat : public ATOSE_file_system
{
public:
	static const uint64_t EOF = ~0;

protected:
	uint64_t base;										// the offset of the FAT-32 volume from the from the start of the physical device
	ATOSE_host_usb_device_disk *disk;

	uint32_t fat_plus;									// true if the mounted volume is a FAT+ volume (see:http://www.fdos.org/kernel/fatplus.txt)
	uint32_t dead_volume;								// true if we can't read this volume and should consider it dead

	uint64_t clusters_on_disk;
	uint64_t reserved_clusters;

	uint64_t root_directory_cluster;
	uint64_t first_data_sector;

	uint64_t bytes_per_sector;
	uint64_t sectors_per_cluster;

protected:
	uint32_t read_sector(void *buffer, uint64_t sector = 0, uint64_t number_of_sectors = 1) { return disk->read_sector(buffer, sector + base, number_of_sectors); }
	uint32_t read_cluster(void *buffer, uint64_t cluster) { return disk->read_sector(buffer, (first_data_sector + (cluster - 2) * sectors_per_cluster) + base, sectors_per_cluster); }
	uint64_t next_cluster_after(uint64_t cluster);
	uint64_t find_in_directory(ATOSE_fat_directory_entry *stats, uint64_t start_cluster, uint8_t *name);
	uint8_t *eight_point_three_to_utf8_strcpy(uint8_t *destination, uint8_t *eight_point_three, uint32_t length);

public:
	ATOSE_fat(ATOSE_host_usb_device_disk *disk, uint64_t base = 0);

	virtual ATOSE_file_control_block *open(ATOSE_file_control_block *fcb, uint8_t *filename);
	virtual ATOSE_file_control_block *close(ATOSE_file_control_block *fcb);

	virtual uint8_t *get_current_block(ATOSE_file_control_block *fcb) { return read_cluster(fcb->buffer, fcb->current_block) == 0 ? fcb->buffer : NULL; }
	virtual uint8_t *get_next_block(ATOSE_file_control_block *fcb);
	virtual uint8_t *get_random_block(ATOSE_file_control_block *fcb, uint64_t bytes_into_file);
} ;

#endif
