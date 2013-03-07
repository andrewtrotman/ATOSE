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
	------------------------
	------------------------
	------------------------
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_hex(int data);
/*
	------------------------
	------------------------
	------------------------
*/


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
read_sector(&boot_sector);

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
root_directory_cluster = boot_sector.BPB_RootClus;
sectors_per_cluster = boot_sector.BPB_SecPerClus;
first_data_sector = boot_sector.BPB_RsvdSecCnt + (boot_sector.BPB_NumFATs * boot_sector.BPB_FATSz32);
bytes_per_sector = boot_sector.BPB_BytsPerSec;
reserved_clusters = boot_sector.BPB_RsvdSecCnt;

/*
	Sanity check (There Ain't No Sanity Clause, Ignite, Nasty... look them up on Google).
*/
/*
	"Count of bytes per sector. This value may take on only
	the following values: 512, 1024, 2048 or 4096"
*/
switch(bytes_per_sector)
	{
	case 512:  break;
	case 1024: break;
	case 2048: break;
	case 4096: break;
	default:   return;
	}
/*
	"Number of sectors per allocation unit. This value must
	be a power of 2 that is greater than 0. The legal values
	are 1, 2, 4, 8, 16, 32, 64, and 128"
*/
switch(sectors_per_cluster)
	{
	case 1: break;
	case 2: break;
	case 4: break;
	case 8: break;
	case 16: break;
	case 32: break;
	case 64: break;
	case 128: break;
	default: return;
	}

/*
	"Note however, that a
	value should never be used that results in a “bytes per
	cluster” value (BPB_BytsPerSec * BPB_SecPerClus ) greater
	than 32K (32 * 1024)"
*/

if ((bytes_per_sector * sectors_per_cluster) > (32 * 1024))
	return;

/*
	Mark the disk as good (so far)
*/
dead_volume = false;
}

/*
	ATOSE_FAT::NEXT_CLUSTER_AFTER()
	-------------------------------
*/
uint64_t ATOSE_fat::next_cluster_after(uint64_t cluster)
{
uint8_t sector[bytes_per_sector];
uint64_t fat_offset = cluster * 4;
uint64_t next_cluster;
uint64_t sector_number;
uint64_t offset;
/*
	"FATSz = BPB_FATSz32;"

	"ThisFATSecNum = BPB_ResvdSecCnt + (FATOffset / BPB_BytsPerSec);"
	"ThisFATEntOffset = REM(FATOffset / BPB_BytsPerSec);"

	"ThisFATSecNum is the sector number of the FAT
	sector that contains the entry for cluster N in the first FAT. If you want the sector
	number in the second FAT, you add FATSz to ThisFATSecNum ; for the third FAT,
	you add 2*FATSz, and so on."
*/
sector_number = reserved_clusters + (fat_offset / bytes_per_sector);
offset = fat_offset % bytes_per_sector;

read_sector(sector, sector_number);

/*
	"A FAT32 FAT entry is actually only a 28- bit
	entry. The high 4 bits of a FAT32 FAT entry are reserved."
*/
next_cluster = (*(uint32_t *)(sector + offset)) & 0x0FFFFFFF;
if (next_cluster >= 0x0FFFFFF8)
	return EOF;

return next_cluster;
}

/*
	ATOSE_FAT::FIND_IN_DIRECTORY()
	------------------------------
	returns the cluster number of the first cluster of "name" in the directory starting at "start_cluster"
	or EOF if it cannot find the name
*/
uint64_t ATOSE_fat::find_in_directory(uint64_t start_cluster, uint8_t *name)
{
uint8_t long_filename[(ATOSE_fat_directory_entry::MAX_LONG_FILENAME_LENGTH + 1) * sizeof(uint16_t)];
uint8_t *into;
uint32_t filename_part;
uint32_t state;
uint8_t sector[bytes_per_sector * sectors_per_cluster];
uint64_t cluster;
ATOSE_fat_directory_entry *file, *end;

filename_part = 1;
into = long_filename;
state = ATOSE_fat_directory_entry::SHORT_FILENAME;
end = (ATOSE_fat_directory_entry *)(sector + bytes_per_sector * sectors_per_cluster);
for (cluster = start_cluster; cluster != EOF; cluster = next_cluster_after(cluster))
	{
	read_cluster(sector, cluster);
	for (file = (ATOSE_fat_directory_entry *)sector; file < end; file++)
		{
		/*
			Check for the end of the list of files in this directory
		*/
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			return EOF;			// not found

		/*
			Check we have a file (and not a deleted file)
		*/
		if (file->DIR_Name[0] != ATOSE_fat_directory_entry::ENTRY_FREE)
			{
			/*
				Check we have a long filename
			*/
			if ((file->DIR_Attr & ATOSE_fat_directory_entry::ATTR_LONG_NAME) == ATOSE_fat_directory_entry::ATTR_LONG_NAME)
				{
				/*
					There are four cases here...
					1. the sequence number is too large
					2. we have the correct next sequence number
					3. the sequence number is (correct | LAST_LONG_ENTRY) in which case we're at the end of a long filename
					4. the sequence number is wrong in which case we have an "orphaned" long filename
				*/
				if (file->LDIR_Ord == filename_part || file->LDIR_Ord  == (ATOSE_fat_directory_entry::LAST_LONG_ENTRY | filename_part))
					{
					/*
						Here we build the long filename
					*/
					memcpy(into, file->LDIR_Name1, sizeof(file->LDIR_Name1));			// case 2
					into += sizeof(file->LDIR_Name1);
					memcpy(into, file->LDIR_Name2, sizeof(file->LDIR_Name2));
					into += sizeof(file->LDIR_Name2);
					memcpy(into, file->LDIR_Name3, sizeof(file->LDIR_Name3));
					into += sizeof(file->LDIR_Name3);

					if (file->LDIR_Ord > ATOSE_fat_directory_entry::LAST_LONG_ENTRY)
						{
						if (file->LDIR_Ord == (ATOSE_fat_directory_entry::LAST_LONG_ENTRY | filename_part))
							{
							state = ATOSE_fat_directory_entry::LONG_FILENAME;			// case 3
							filename_part = 0;
							}
						else
							{
							debug_print_string("Too large\r\n");
							debug_print_this("Expected:", filename_part);
							debug_print_this("Got:", file->LDIR_Ord);

							state = ATOSE_fat_directory_entry::ORPHAN_FILENAME;		// case 1
							}
						}
					filename_part++;
					}
				else
					{
					debug_print_string("Out of sequence\r\n");
					debug_print_this("Expected:", filename_part);
					debug_print_this("Got:", file->LDIR_Ord);
					state = ATOSE_fat_directory_entry::ORPHAN_FILENAME;		// case 4
					}
				}
			else
				{
				/*
					We now find ourselves with a short filename which is either a true short filename or a surrogate for the long filename
				*/
				if (state == ATOSE_fat_directory_entry::LONG_FILENAME)
					debug_dump_buffer(long_filename, 0, (into - long_filename) * 2);
				else if (state == ATOSE_fat_directory_entry::ORPHAN_FILENAME)
					debug_print_string("--ORPHAN FILE--");
				else if (state == ATOSE_fat_directory_entry::SHORT_FILENAME)
					{
					uint8_t name[13];
					memset(name, ' ', sizeof(name));
					memcpy(name, file->DIR_Name, 8);
					name[8] = '.';
					memcpy(name + 9, file->DIR_Extension, 3);
					name[12] = '\0';

					debug_print_string((char *)name);
					}
//				else	// there is no else case

				debug_print_string("\r\n");

				filename_part = 1;
				into = long_filename;
				state = ATOSE_fat_directory_entry::SHORT_FILENAME;
				}
			}
		}
	}
return EOF;		// not found
}

/*
	ATOSE_FAT::DIR()
	----------------
*/
void ATOSE_fat::dir(void)
{
uint8_t sector[bytes_per_sector * sectors_per_cluster];
uint8_t name[13];
ATOSE_fat_directory_entry *file;
uint32_t files_per_sector = sizeof(sector) / sizeof(*file);
uint32_t current;
uint64_t cluster;

cluster = root_directory_cluster;
for (cluster = root_directory_cluster; cluster != EOF; cluster = next_cluster_after(cluster))
	{
	debug_print_string("                                 READ\r\n");
	memset(sector, 'X', sizeof(cluster));
	read_cluster(sector, cluster);
	debug_dump_buffer(sector, 0, sizeof(sector));

	file = (ATOSE_fat_directory_entry *)sector;
	for (current = 0; current < files_per_sector; current++)
		{
		if (*file[current].DIR_Name == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			break;

		if (*file[current].DIR_Name != ATOSE_fat_directory_entry::ENTRY_FREE)
			{
			if (*file[current].DIR_Name == ATOSE_fat_directory_entry::ENTRY_USE_0xE5)
				*file[current].DIR_Name = ATOSE_fat_directory_entry::ENTRY_FREE;

			if ((file[current].DIR_Attr & ATOSE_fat_directory_entry::ATTR_LONG_NAME) == ATOSE_fat_directory_entry::ATTR_LONG_NAME)
				debug_print_string("L ");
			else
				debug_print_string("S ");

			memset(name, ' ', sizeof(name));
			memcpy(name, file[current].DIR_Name, 8);
			name[8] = '.';
			memcpy(name + 9, file[current].DIR_Extension, 3);
			name[12] = '\0';
			debug_print_string((char *)name);
			debug_print_string("\r\n");
			}
		}


find_in_directory(root_directory_cluster, (uint8_t *)"ATOSE");
}
}
