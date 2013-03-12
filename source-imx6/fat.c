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
#include "usc2_str.h"
#include "file_control_block.h"

/*
	------------------------
	------------------------
	------------------------
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_cf_this(const char *start, uint32_t hex, uint32_t second, const char *end = "");
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
reserved_sectors = boot_sector.BPB_RsvdSecCnt;
fats_on_volume = boot_sector.BPB_NumFATs;
sectors_per_fat = boot_sector.BPB_FATSz32;

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
	Not there yet... check for FAT+ (see:http://www.fdos.org/kernel/fatplus.txt)
*/
if (boot_sector.BPB_FSVer == 0x0000)
	fat_plus = false;
else if (boot_sector.BPB_FSVer == 0x0010)
	fat_plus = true;
else
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
sector_number = reserved_sectors + (fat_offset / bytes_per_sector);
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
	ATOSE_FAT::EIGHT_POINT_THREE_TO_UTF8_STRCPY()
	---------------------------------------------
*/
uint8_t *ATOSE_fat::eight_point_three_to_utf8_strcpy(uint8_t *destination, uint8_t *eight_point_three, uint32_t length)
{
uint8_t *spacer;

if (length < 13)
	return NULL;

memcpy(destination, eight_point_three, 8);
for (spacer = destination + 7; spacer >= destination; spacer--)
	if (!ASCII_isspace(*spacer))
		break;

spacer++;
*spacer++ = '.';

memcpy(spacer, eight_point_three + 8, 3);
spacer[4] = '\0';
if (spacer[3] == ' ')
	{
	spacer[3] = '\0';
	if (spacer[2] == ' ')
		{
		spacer[2] = '\0';
		if (spacer[1] == ' ')
			spacer[1] = '\0';
		}
	}
return destination;
}

/*
	ATOSE_FAT::OPEN()
	-----------------
*/
ATOSE_file_control_block *ATOSE_fat::open(ATOSE_file_control_block *fcb, uint8_t *filename)
{
uint32_t first_block;
ATOSE_fat_directory_entry found_file;

if ((first_block = find_in_directory(&found_file, root_directory_cluster, filename)) == EOF)
	return NULL;

fcb->file_system = this;
fcb->first_block = first_block;
fcb->block_size_in_bytes = bytes_per_sector * sectors_per_cluster;

fcb->current_block = first_block;
fcb->buffer = NULL;
fcb->file_offset = 0;

/*
	FAT+ (see: http://www.fdos.org/kernel/fatplus.txt)
	"This means bits 0-2 and 5-7 are reserved, so these 6 bits are used by a FAT+ file system to store
	the upper 6 bits of a file's size when the size becomes or exceeds 4GiB, giving 38 bits instead
	of just 32 to store the size. This allows for a file up to 274877906944 bytes (256GiB)."
*/
fcb->file_size_in_bytes = found_file.DIR_FileSize;
if (fat_plus)
	fcb->file_size_in_bytes += ((uint64_t)(found_file.DIR_NTRes & 0x07)) + ((((uint64_t)found_file.DIR_NTRes >> 5) << 3) << 32);

return fcb;
}

/*
	ATOSE_FAT::CLOSE()
	------------------
	No work necessary for FAT32 - so we just succeed
*/
ATOSE_file_control_block *ATOSE_fat::close(ATOSE_file_control_block *fcb)
{
return fcb;
}

/*
	ATOSE_FAT::GET_NEXT_BLOCK()
	---------------------------
*/
uint8_t *ATOSE_fat::get_next_block(ATOSE_file_control_block *fcb)
{
if (fcb->current_block == EOF)
	return NULL;

if ((fcb->current_block = next_cluster_after(fcb->current_block)) == EOF)
	return NULL;

if (read_cluster(fcb->buffer, fcb->current_block) == 0)
	return fcb->buffer;

return NULL;
}

/*
	ATOSE_FAT::WRITE_CURRENT_BLOCK()
	--------------------------------
	The FCB contains both the block to write (fcb->buffer) and the current block number (fcb->current_block)
	Here we really only need to write the block back to where it came from
*/
uint8_t *ATOSE_fat::write_current_block(ATOSE_file_control_block *fcb)
{
/*
	We don't write past EOF
*/
if (fcb->current_block == EOF)
	return NULL;

if (write_cluster(fcb->buffer, fcb->current_block) == 0)
	return fcb->buffer;

return NULL;
}

/*
	ATOSE_FAT::GET_RANDOM_BLOCK()
	-----------------------------
*/
uint8_t *ATOSE_fat::get_random_block(ATOSE_file_control_block *fcb)
{
uint64_t block_number, current_block, block_count;

block_number = fcb->file_offset / fcb->block_size_in_bytes;
current_block = fcb->first_block;

/*
	Seek to the cluster
*/
for (block_count = 0; block_count < block_number; block_count++)
	if ((current_block = next_cluster_after(current_block)) == EOF)
		return NULL;

/*
	Read the cluster
*/
fcb->current_block = current_block;
if (read_cluster(fcb->buffer, fcb->current_block) == 0)
	return fcb->buffer;

return NULL;
}

/*
	ATOSE_FAT::EXTEND()
	-------------------
	return 0 on success, and other value is an error:
	SUCCESS = 0
	ERROR_FILE_SIZE_EXCEEDED
	ERROR_DISK_FULL
	ERROR_BAD_FILE
	it also return lower-level media errors
*/
uint64_t ATOSE_fat::extend(ATOSE_file_control_block *fcb, uint64_t length_to_become)
{
int64_t first_free_fat_sector;
uint64_t bytes_per_cluster;
uint64_t current, fat_sector, free_clusters;
uint64_t next_cluster, last_cluster_in_file;
uint64_t length_of_file_on_disk_in_clusters, fat_copy;
uint64_t entries_per_sector = bytes_per_sector / sizeof(uint32_t);
uint64_t clusters_needed, clusters_added, new_cluster;
uint64_t error;
uint32_t sector[bytes_per_sector];
uint8_t blank_cluster[bytes_per_sector * sectors_per_cluster];

/*
	Check we are not exceeding file system limits
*/
if (length_to_become > 0xFFFFFFFF)					// 32 bits
	{
	if (!fat_plus)
		return ERROR_FILE_SIZE_EXCEEDED;
	if (length_to_become > 0x3FFFFFFFFF)			// 38 bits, see: http://www.fdos.org/kernel/fatplus.txt
		return ERROR_FILE_SIZE_EXCEEDED;
	}

/*
	Are we actually making the file longer?
*/
bytes_per_cluster = bytes_per_sector * sectors_per_cluster;
clusters_needed = (length_to_become / bytes_per_cluster) - (fcb->file_size_in_bytes / bytes_per_cluster);
if (clusters_needed <= 0)
	return SUCCESS;		// success (because we are already long enough

/*
	We're allowed to make the file this long (i.e. it does not exceed file system limits)
	But there might not be enough disk space.  So we check the FAT has space in it... this,
	unfortunately, is a linear search through the FAT.
*/
first_free_fat_sector = -1;
free_clusters = 0;
for (fat_sector = 0; fat_sector < sectors_per_fat && free_clusters < clusters_needed; fat_sector++)
	{
	/*
		Get the FAT sector
	*/
	if ((error = read_sector(sector, reserved_sectors + fat_sector)) != 0)
		return error;

	/*
		Walk each entry in the FAT looking for empties
	*/
	for (current = 0; current < entries_per_sector && free_clusters < clusters_needed; current++)
		{
		if (fat_sector * entries_per_sector > clusters_on_disk)
			return ERROR_DISK_FULL;		// We've walked off the end of the disk and so we've failed.

		/*
			"A FAT32 FAT entry is actually only a 28- bit entry"
		*/
		if ((sector[current] & 0x0FFFFFFF) == 0)
			{
			if (first_free_fat_sector < 0)
				first_free_fat_sector = fat_sector;
			free_clusters++;
			}
		}
	}

/*
	Find the last cluster in the file (because that is where we're going to update from
*/
next_cluster = last_cluster_in_file = fcb->current_block;
length_of_file_on_disk_in_clusters = 0;
do
	{
	last_cluster_in_file = next_cluster;
	next_cluster = next_cluster_after(last_cluster_in_file);

	/*
		Although I've not see this, its possible for the FAT to form a circle in which case we
		might end up looping forever so we check for that and quit if we find it happening.
	*/
	length_of_file_on_disk_in_clusters++;
	if (length_of_file_on_disk_in_clusters * bytes_per_cluster > length_to_become)
		return ERROR_BAD_FILE;
	}
while (next_cluster != EOF);

/*
	At this point we can guarantee that we can, indeed, extend the file to the required length
	some stuff might still go wrong, but those things are probably catastrophic (e.g. eject the
	disk mid write, or a write failure, etc.).  We also know the last cluster in the file, and that
	the file chain is (at least at the end) correct
*/
memset(blank_cluster, 0, bytes_per_sector * sectors_per_cluster);
clusters_added = 0;
for (fat_sector = first_free_fat_sector; fat_sector < sectors_per_fat && clusters_added < clusters_needed; fat_sector++)
	{
	/*
		Get the FAT sector then find a free cluster in it
	*/
	if ((error = read_sector(sector, reserved_sectors + fat_sector)) != 0)
		return error;

	for (current = 0; current < entries_per_sector && clusters_added < clusters_needed; current++)
		{
		if ((sector[current] & 0x0FFFFFFF) == 0)
			{
			/*
				At this point we have the new cluster new_cluster = (fat_sector * (bytes_per_sector / sizeof(uint32_t)))+ current
				we also have the cluster id of the end of the file (current_block)
				so we write EOF to the new_cluster to identify it as being at EOF
			*/
			sector[current] = EOC;

			/*
				Update *all* copies of the FAT (there might be several, but we expect only 2)
				"If you want the sector number in the second FAT, you add FATSz to ThisFATSecNum;
				for the third FAT, you add 2*FATSz, and so on."
			*/
			for (fat_copy = 0; fat_copy < fats_on_volume; fat_copy++)
				if ((error = write_sector(sector, reserved_sectors + fat_sector + (sectors_per_fat * fat_copy))) != 0)
					return error;

			/*
				Now we update the end of file to point to the new end of file
				compute the update, load the FAT entry, update it, write it back to all copies of the FAT
			*/
			if ((error = read_sector(sector, reserved_sectors + last_cluster_in_file / entries_per_sector)) != 0)
				return error;

			new_cluster = fat_sector * entries_per_sector + current;
			sector[last_cluster_in_file % entries_per_sector] = new_cluster;

			for (fat_copy = 0; fat_copy < fats_on_volume; fat_copy++)
				if ((error = write_sector(sector, reserved_sectors + last_cluster_in_file / entries_per_sector)) != 0)
					return error;

			last_cluster_in_file = new_cluster;
			clusters_added++;

			/*
				I guess we should make sure its blank
			*/
			if ((error = write_cluster(blank_cluster, new_cluster)) != 0)
				return error;
			}
		}
	}

/*
	At this point we've updated all the FATs with the new chains, next we need to update the directory
*/
fcb->file_size_in_bytes = length_to_become;
return set_file_properties(root_directory_cluster, fcb->first_block, length_to_become);
}

/*
	ATOSE_FAT::FIND_IN_DIRECTORY()
	------------------------------
	returns the cluster number of the first cluster of "name" in the directory starting at "start_cluster"
	or EOF if it cannot find the name
*/
uint64_t ATOSE_fat::find_in_directory(ATOSE_fat_directory_entry *stats, uint64_t start_cluster, uint8_t *name)
{
uint8_t long_filename[(ATOSE_fat_directory_entry::MAX_LONG_FILENAME_LENGTH + 1) * sizeof(uint16_t)];
uint8_t utf8_long_filename[sizeof(long_filename)];
uint8_t *into;
uint32_t filename_part = 1;
uint8_t cluster[bytes_per_sector * sectors_per_cluster];
uint64_t cluster_id;
ATOSE_fat_directory_entry *file, *end;

end = (ATOSE_fat_directory_entry *)(cluster + bytes_per_sector * sectors_per_cluster);
for (cluster_id = start_cluster; cluster_id != EOF; cluster_id = next_cluster_after(cluster_id))
	{
	read_cluster(cluster, cluster_id);
	for (file = (ATOSE_fat_directory_entry *)cluster; file < end; file++)
		{
		/*
			Check for the end of the list of files in this directory
		*/
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			return EOF;			// not found

		/*
			Check we have a file (and not a deleted file)
		*/
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_FREE)
			{
			filename_part = ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER  + 1;
			continue;
			}

		/*
			Check if we have a long filename
		*/
		if ((file->DIR_Attr & ATOSE_fat_directory_entry::ATTR_LONG_NAME) == ATOSE_fat_directory_entry::ATTR_LONG_NAME)
			{
			/*
				The sequence numbers are in descending order starting with ATOSE_fat_directory_entry::LAST_LONG_ENTRY | n where n is the number of
				pieces the name is broken into.  We have to watch out for this:
					"Names are also NUL terminated and padded with 0xFFFF
					characters in order to detect corruption of long name fields by errant disk utilities.
					A name that fits exactly in a n long directory entries (i.e. is an integer multiple of
					13) is not NUL terminated and not padded with 0xFFFFs."
			*/
			if ((file->LDIR_Ord & ATOSE_fat_directory_entry::LAST_LONG_ENTRY) != 0)
				{
				/*
					We're at the start of a long filename (whose sequence numbers must descend).
				*/
				filename_part = file->LDIR_Ord & ~ATOSE_fat_directory_entry::LAST_LONG_ENTRY;
				/*
					Check the sequence number isn't too high
				*/
				if (filename_part > ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER)
					continue;
				/*
					As above, we must '\0' terminate ourselves
				*/
				memset(long_filename, 0, sizeof(long_filename));
				}
			/*
				Check the sequence number is the next in the sequence
			*/
			if (filename_part != (file->LDIR_Ord & ~ATOSE_fat_directory_entry::LAST_LONG_ENTRY))
				{
				filename_part = ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER  + 1;
				continue;
				}

			/*
				Build the long filename
			*/
			into = long_filename + ((filename_part - 1) * ATOSE_fat_directory_entry::LAST_LONG_BYTES_PER_SHORT_NAME);

			memcpy(into, file->LDIR_Name1, sizeof(file->LDIR_Name1));
			into += sizeof(file->LDIR_Name1);
			memcpy(into, file->LDIR_Name2, sizeof(file->LDIR_Name2));
			into += sizeof(file->LDIR_Name2);
			memcpy(into, file->LDIR_Name3, sizeof(file->LDIR_Name3));
			into += sizeof(file->LDIR_Name3);
			/*
				set the sequence number to the one we expect next
			*/
			filename_part--;
			}
		else
			{
			/*
				We now find ourselves with a short filename which is either a true short filename or a surrogate for the long filename
				Either way, there's some translation necessary
			*/
			if (filename_part == 0)
				UCS2_to_utf8_strcpy(utf8_long_filename, (uint16_t *)long_filename, sizeof(utf8_long_filename));
			else
				eight_point_three_to_utf8_strcpy(utf8_long_filename, file->DIR_Name, sizeof(utf8_long_filename));

			/*
				We got the name, now do the comparison (in UTF-8)
			*/
			if (ASCII_strcmp((char *)utf8_long_filename, (char *)name) == 0)
				{
				memcpy(stats, file, sizeof(*stats));
				return ((uint64_t)file->DIR_FstClusHI << 16) | (uint64_t)file->DIR_FstClusLO;
				}

			/*
				Move on to the next file
			*/
			filename_part = 1;
			}
		}
	}
return EOF;		// not found
}

/*
	ATOSE_FAT::SET_FILE_PROPERTIES()
	--------------------------------
	Given a starting cluster for a file (i.e. it's id), set the attributes of that file to match those passed
	at first we'll only worry about file size.

	return 0 on success, EOF on can't find file, all other values are errors
*/
uint64_t ATOSE_fat::set_file_properties(uint64_t directory_start_cluster, uint64_t file_start_cluster, uint64_t filesize)
{
uint32_t entries_per_cluster = (bytes_per_sector * sectors_per_cluster) / sizeof(ATOSE_fat_directory_entry);
ATOSE_fat_directory_entry cluster[entries_per_cluster];
uint64_t cluster_id;
ATOSE_fat_directory_entry *file;

for (cluster_id = directory_start_cluster; cluster_id != EOF; cluster_id = next_cluster_after(cluster_id))
	{
	if (read_cluster(cluster, cluster_id) != 0)
		return 0;
	for (file = cluster; file < cluster + entries_per_cluster; file++)
		{
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			return EOF;			// We're at the end of the list of files so we're not found
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_FREE)
			continue;			// not a file, so ignore
		if ((file->DIR_Attr & ATOSE_fat_directory_entry::ATTR_LONG_NAME) == ATOSE_fat_directory_entry::ATTR_LONG_NAME)
			continue;		// long filename so ignore

		if (file->DIR_FstClusHI == ((file_start_cluster >> 16) & 0xFFFF) && (file->DIR_FstClusLO == (file_start_cluster & 0xFFFF)))
			{
			/*
				We found it... yay!  Now we set the file size.  In both FAT and FAT+ we store the low 32 bits in DIR_FileSize
			*/
			file->DIR_FileSize = filesize & 0xFFFFFFFF;
			if (fat_plus)
				{
				/*
					FAT+ stores the top 6 bits of the file size (to a total of 38 bits) in bits 0-2 and 5-6 of DIR_NTRes
				*/
				file->DIR_NTRes &= ((1 << 3) | (1 << 4));		// leave the middle two bits in tact
				file->DIR_NTRes |= (filesize >> 32) & ((1 << 0) | (1 << 1) | (1 << 2));		// bottom 3 bits of DIR_NTRes are bits 32,33,34 of the file size
				file->DIR_NTRes |= ((filesize >> 35) & ((1 << 0) | (1 << 1))) << 5;
				}

			return write_cluster(cluster, cluster_id);
			}
		}
	}
return EOF;		// not found
}
