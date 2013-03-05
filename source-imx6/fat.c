/*
	FAT.C
	-----
	At present we only support FAT-32.  Its implemented according to
	"Microsoft Extensible Firmware Initiative FAT32 File System Specification FAT: General Overview of On- Disk Format Version 1.03, December 6, 2000 Microsoft Corporation".
	Which was sourced from the thinFAT32 project. I did not use the thinFAT32 code because the structures were
	not correctly packed, they broke from the naming convention in the spec, and the code caused alignment falures.
	The code here is a re-implementation supporting only FAT32
*/
#include <stdint.h>
#include "fat.h"
#include "fat_directory_entry.h"
#include "fat_boot_sector.h"
#include "ascii_str.h"

/*
	ATOSE_FAT::ATOSE_FAT()
	----------------------
*/
ATOSE_fat::ATOSE_fat(ATOSE_host_usb_device_disk *disk, uint64_t base)
{
ATOSE_fat_boot_sector boot_sector;
uint64_t data_sectors;

dead_volume = true;

this->disk = disk;
this->base = base;
read_sector(&boot_sector, 0);


/*
	Check that we have a BPB
*/
/*
	"Jump instruction to boot code. This field has two allowed forms:
	jmpBoot[0] = 0xEB, jmpBoot[1] = 0x??, jmpBoot[2] = 0x90
	and
	jmpBoot[0] = 0xE9, jmpBoot[1] = 0x??, jmpBoot[2] = 0x??
	0x?? indicates that any 8- bit value is allowed in that byte"
*/
if (!((boot_sector.BS_jmpBoot[0] == 0xEB && boot_sector.BS_jmpBoot[2] == 0x90) || (boot_sector.BS_jmpBoot[0] == 0xE9)))
	return;
/*
	"The legal values for this field are 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, and 0xFF"
*/
switch (boot_sector.BPB_Media)
	{
	case 0xF0: break;			// "for removable media, 0xF0 is frequently used."
	case 0xF8: break;			// "standard value for “fixed” (non- removable)"
	case 0xF9: break;
	case 0xFA: break;
	case 0xFB: break;
	case 0xFC: break;
	case 0xFD: break;
	case 0xFE: break;
	case 0xFF: break;
	default:
		return;
	}

/*
	"This field is the FAT12/FAT16 16- bit count of sectors
	occupied by ONE FAT. On FAT32 volumes this field must
	be 0, and BPB_FATSz32 contains the FAT size count."
*/
if (boot_sector.BPB_FATSz16 != 0)
	return;

/*
	"Always set to the string "FAT32   ""
*/
if (ASCII_strncasecmp(boot_sector.BS_FilSysType, "FAT32   ", 8) != 0)
	return;

/*
	"There is one other impor tant note about Sector 0 of a FAT volume. If we consider
	the content s of the sector as a byte array, it must be true that sector[510] equals
	0x55, and sector[511] equals 0xAA."
*/
if (boot_sector.should_be_0x55 != 0x55 || boot_sector.should_be_0xAA != 0xAA)
	return;

/*
	If we get to this point the we think we're a valid FAT32 volume.  However, as there is little
	point in supporting the old FAT12 or FAT16 formatted FAT32 volumes, we'll reject those too
*/
/*
	"Note that on a FAT32 volume the BPB_RootEntCnt value is always 0"
*/
if (boot_sector.BPB_RootEntCnt != 0)
	return;

/*
	"For FAT32 volumes, this field must be 0."
*/
if (boot_sector.BPB_TotSec16 != 0)
	return;

/*
	"First, we determine the count of sectors occupied by the root directory"

	RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec;

	"Note that on a FAT32 volume, the BPB_RootEntCnt value is always 0; so on a FAT32
	volume, RootDirSectors is always 0."

	"Next, we determine the count of sectors in the data region of the volume:"
*/

data_sectors = boot_sector.BPB_TotSec32 - (boot_sector.BPB_RsvdSecCnt + boot_sector.BPB_NumFATs * boot_sector.BPB_FATSz32);

/*
	Now we determine the count of clusters:
*/
clusters_on_disk = data_sectors / boot_sector.BPB_SecPerClus;

/*
	"Now we can determine the FAT type"
*/
if (clusters_on_disk < 4085)
	return; /* Volume is FAT12 */
else if(clusters_on_disk < 65525)
	return; /* Volume is FAT16 */
//else  /* Volume is FAT32 */

/*
	Now we extract some goodies from the BPB
*/
root_directory_location = boot_sector.BPB_RootClus;

dead_volume = false;
}

/*
	ATOSE_FAT::DIR()
	----------------
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_hex(int data);

void ATOSE_fat::dir(void)
{
uint8_t name[13];
uint8_t sector[512];
ATOSE_fat_directory_entry *file;
uint32_t files_per_sector = sizeof(sector) / sizeof(*file);
uint32_t current;

read_sector(sector, root_directory_location);
file = (ATOSE_fat_directory_entry *)sector;

for (current = 0; current < files_per_sector; current++)
	{
	memset(name, ' ', sizeof(name));
	memcpy(name, file[current].DIR_Name, 8);
	name[8] = '.';
	memcpy(name + 9, file[current].DIR_Extension, 3);
	name[12] = '\0';
	debug_print_string((char *)name);
	debug_print_string("\r\n");
	}
}
