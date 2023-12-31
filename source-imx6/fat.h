/*
	FAT.H
	-----
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef FAT_H_
#define FAT_H_

#include <stdint.h>
#include "host_usb_device_disk.h"
#include "file_system.h"
#include "file_control_block.h"
#include "fat_directory_entry.h"

/*
	class ATOSE_FAT
	---------------
*/
class ATOSE_fat : public ATOSE_file_system
{
protected:
	enum {RETURN_FIRST_BLOCK, DELETE_FILE, SET_FILE_ATTRIBUTES };

public:
	static const uint64_t EOF = ~0;
	static const uint32_t EOC = 0x0FFFFFFF;
	static const uint64_t FILE_SIZE_DELETE_KEY = 0xFFFFFFFFFFFFFFFF;
	static const uint32_t MAX_SHORTNAME_RETRIES = 1000;		// must be smaller than 999999

protected:
	uint64_t base;											// the offset of the FAT-32 volume from the from the start of the physical device
	ATOSE_host_usb_device_disk *disk;

	uint32_t fat_plus;									// true if the mounted volume is a FAT+ volume (see:http://www.fdos.org/kernel/fatplus.txt)
	uint32_t fats_on_volume;							// when we extend a file we need to make sure we update the primary and all duplicate FATs
	uint32_t sectors_per_fat;

	uint64_t clusters_on_disk;
	uint64_t reserved_sectors;

	uint64_t root_directory_cluster;
	uint64_t first_data_sector;

	uint64_t fsinfo_sector;								// location of the BPB_FSInfo structure
	uint64_t free_cluster_count;						// FSI_Free_Count

	uint64_t bytes_per_sector;
	uint64_t sectors_per_cluster;
	uint64_t bytes_per_cluster;						// bytes_per_sector * sectors_per_cluster

protected:
	/*
		FAT+ (see: http://www.fdos.org/kernel/fatplus.txt)
		"This means bits 0-2 and 5-7 are reserved, so these 6 bits are used by a FAT+ file system to store
		the upper 6 bits of a file's size when the size becomes or exceeds 4GiB, giving 38 bits instead
		of just 32 to store the size. This allows for a file up to 274877906944 bytes (256GiB)."
	*/
	uint64_t get_filesize(uint64_t DIR_FileSize, uint64_t DIR_NTRes) { return DIR_FileSize + (fat_plus ? ((DIR_NTRes & 0x07) + (((DIR_NTRes >> 5) << 3) << 32)) : 0); }
	uint64_t set_filesize(uint32_t *DIR_FileSize, uint8_t *DIR_NTRes, uint64_t filesize);
	uint32_t write_fsinfo(void);

	uint64_t find_in_directory(ATOSE_fat_directory_entry *stats, uint64_t start_cluster, const uint8_t *name, uint32_t action = RETURN_FIRST_BLOCK, void *value = 0);

	uint32_t read_sector(void *buffer, uint64_t sector = 0, uint64_t number_of_sectors = 1) { return disk->read_sector(buffer, sector + base, number_of_sectors); }
	uint32_t write_sector(void *buffer, uint64_t sector = 0, uint64_t number_of_sectors = 1) { return disk->write_sector(buffer, sector + base, number_of_sectors); }
	uint32_t read_cluster(void *buffer, uint64_t cluster)  { return disk->read_sector(buffer, (first_data_sector + (cluster - 2) * sectors_per_cluster) + base, sectors_per_cluster); }
	uint32_t write_cluster(void *buffer, uint64_t cluster) { return disk->write_sector(buffer, (first_data_sector + (cluster - 2) * sectors_per_cluster) + base, sectors_per_cluster); }
	uint64_t next_cluster_after(uint64_t cluster);

	uint8_t shortname_checksum(uint8_t *name);
	uint8_t *eight_point_three_to_utf8_strcpy(uint8_t *destination, uint8_t *eight_point_three, uint32_t length);
	uint8_t *short_name_from_long_name(uint8_t *short_name, uint16_t *long_name);
	uint32_t increment_short_filename(uint8_t *filename, uint32_t last);
	uint32_t long_filename_to_pieces(ATOSE_fat_directory_entry *into, uint16_t *filename, uint8_t checksum);

	uint32_t allocate_free_cluster(uint64_t last_cluster_in_file, uint64_t *head_of_chain = 0, uint64_t clusters_needed = 1, uint64_t *clusters_allocated = 0);
	uint64_t add_to_directory(uint64_t directory_start_cluster, ATOSE_fat_directory_entry *new_filename, uint32_t parts);

public:
	ATOSE_fat() : ATOSE_file_system() {}
	void initialise(ATOSE_host_usb_device_disk *disk, uint64_t base = 0);

	virtual ATOSE_file_control_block *create(ATOSE_file_control_block *fcb, uint8_t *filename);
	virtual ATOSE_file_control_block *open(ATOSE_file_control_block *fcb, const uint8_t *filename);
	virtual ATOSE_file_control_block *close(ATOSE_file_control_block *fcb);
	virtual uint64_t extend(ATOSE_file_control_block *fcb, uint64_t new_length);
	virtual uint32_t unlink(uint8_t *filename);

	virtual uint8_t *get_next_block(ATOSE_file_control_block *fcb);
	virtual uint8_t *get_random_block(ATOSE_file_control_block *fcb);
	virtual uint8_t *write_current_block(ATOSE_file_control_block *fcb);
} ;

#endif
