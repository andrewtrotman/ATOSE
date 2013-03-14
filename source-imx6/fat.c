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
#include "utf8_str.h"
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
	check for zero-length files
*/
if (cluster == 0)
	return EOF;

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

/*
	Copy the 8-character name and remove spaces from the end
*/
memcpy(destination, eight_point_three, 8);
for (spacer = destination + 7; spacer >= destination; spacer--)
	if (!ASCII_isspace(*spacer))
		break;

/*
	Put a do into it
*/
spacer++;
*spacer++ = '.';

/*
	Copy the 3 character extension and remove spaces from the end
*/
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

/*
	Make sure the filename isn't too long
*/
if (ASCII_strlen(filename) > sizeof(fcb->filename))
	return NULL;

/*
	Find the directory entry (if there is one)
*/
if ((first_block = find_in_directory(&found_file, root_directory_cluster, filename)) == EOF)
	return NULL;

/*
	Create the FCB
*/
ASCII_strcpy(fcb->filename, filename);

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
	ATOSE_FAT::SHORT_NAME_FROM_LONG_NAME()
	--------------------------------------
	FAT uses USC2 encoding... since we must generate that for the long name anyway, we can do that before we generate the short name.  This
	makes it a whole lot easier to generate the short name because we have an array of 16-bit characters rather than a UTF-8 string.

	It really doesn't matter much how we do this because no-one is likely to see the short names, but we'll stick with something reasonable...

	We're going to:

	1. strip out all non-ASCII (we'll only keep 'a'..'z', '0..9')
	2. convert all ASCII characters to upper case
	3. take the first 3 characters after the final "dot"
	then do numeric tail generation.

	Note that this is quite different from the Microsoft algorithms that is:

	"1. The UNICODE name passed to the file system is converted to upper case.
	2. The upper cased UNICODE name is converted to OEM.
	if (the uppercased UNICODE glyph does not exist as an OEM glyph in the OEM code page) or (the OEM glyph is invalid in an 8.3 name)
		{
		Replace the glyph to an OEM '_' (underscore) character.
		Set a "lossy conversion" flag.
		}
	3. Strip all leading and embedded spaces from the long name.
	4. Strip all leading periods from the long name.
	5. While (not at end of the long name) and (char is not a period) and (total chars copied < 8)
		{
		Copy characters into primary portion of the basis name
		}
	6. Insert a dot at the end of the primary component s of the basis- name iff the basis name has an extension after the last period in the name.
	7. Scan for the last embedded period in the long name.
		If (the last embedded period was found)
			{
			While (not at end of the long name) and (total chars copied < 3)
				{
				Copy characters into extension portion of the basis name
				}
			}
	Proceed to numeric - tail generation.
	The Numeric - Tail Generation Algorithm
	If (a "lossy conversion" was not flagged) and (the long name fits within the 8.3 naming conventions) and (the basis - name does not collide with any existing short name)
		{
		The short name is only the basis - name without the numeric tail.
		}
	else
		{
		Insert a numeric- tail "~n" to the end of the primary name such that the value of
		the "~n" is chosen so that the name thus formed does not collide with any existing short name and that the
		primary name does not exceed eight characters in length.
		}"
*/
uint8_t *ATOSE_fat::short_name_from_long_name(uint8_t *short_name, uint16_t *long_name)
{
uint32_t byte;
uint16_t *from, *last_dot = NULL;
uint8_t *to;

/*
	start with all spaces
*/
memset(short_name, ' ', 11);

/*
	copy the name
*/
to = short_name;
from = long_name;
byte = 0;
while (byte < 8)
	{
	if (*from == '\0')
		break;
	else if (ASCII_isalnum(*from))
		{
		*to++ = ASCII_toupper(*from);
		byte++;
		}
	else if (*from == '.')
		last_dot = from;
	from++;
	}

/*
	Keep looking until we find at dot (or end of string)
*/
while (*from != '\0')
	{
	if (*from == '.')
		last_dot = from;
	from++;
	}

/*
	copy the extension
*/
byte = 0;
if (last_dot != NULL)
	while (byte < 3)
		{
		if (*last_dot == '\0')
			break;
		else if (ASCII_isalnum(*last_dot))
			{
			*to++ = ASCII_toupper(*last_dot);
			byte++;
			}
		last_dot++;
		}

return short_name;
}

/*
	ATOSE_FAT::INCREMENT_SHORT_FILENAME()
	-------------------------------------
	In the unfortunate case that a short filename generated with short_name_from_long_name() results in
	a short filename already in use, we need to generate an alternative short filename.  Microsoft do this
	thus:

	"The "~n" string can range from "~1" to "~999999". The number "n" is chosen so
	that it is the next number in a sequence of files with similar basis - names. For
	example, assume the following short names existed: LETTER~1.DOC and
	LETTER~2.DOC. As expected, the next auto- generated name of name of this type
	would be LETTER~3.DOC."

	return 0 on success
*/
uint32_t ATOSE_fat::increment_short_filename(uint8_t *filename, uint32_t last)
{
uint8_t tilde = 7;

filename[tilde--] = '0' + (last % 10);
if (last > 9)
	{
	filename[tilde--] = '0' + (last / 10) % 10;
	if (last > 99)
		{
		filename[tilde--] = '0' + (last / 100) % 10;
		if (last > 999)
			{
			filename[tilde--] = '0' + (last / 1000) % 10;
			if (last > 9999)
				{
				filename[tilde--] = '0' + (last / 10000) % 10;
				if (last > 99999)
					{
					filename[tilde--] = '0' + (last / 100000) % 10;
					if (last > 999999)
						filename[tilde--] = '0' + (last / 1000000) % 10;
					}
				}
			}
		}
	}

filename[tilde] = '~';

return last;
}

/*
	ATOSE_FAT::LONG_FILENAME_TO_PIECES()
	------------------------------------
	Given the long filename in UTF-32, turn it into some number of consecutive FAT long filename pieces and
	return the number we used.

	Note that filename must be 0x00 terminated and after that 0xFF padded to the next 13-character boundary
*/
uint32_t ATOSE_fat::long_filename_to_pieces(ATOSE_fat_directory_entry *into, uint16_t *filename, uint8_t checksum)
{
ATOSE_fat_directory_entry *current;
uint32_t filename_length, needed, sequence_number;
uint16_t *from;

/*
	Find out how long the filename is and therefore how many pieces we'll need, and therefore the first sequence number
*/
filename_length = UCS2_strlen(filename);
needed = (filename_length + ATOSE_fat_directory_entry::LONG_FILENAME_CHARACTERS_PER_ENTRY - 1) / ATOSE_fat_directory_entry::LONG_FILENAME_CHARACTERS_PER_ENTRY;

/*
	Now we count backwards starting with end of the string first (weird hu?).
*/
from = filename + ((filename_length + ATOSE_fat_directory_entry::LONG_FILENAME_CHARACTERS_PER_ENTRY - 1) / ATOSE_fat_directory_entry::LONG_FILENAME_CHARACTERS_PER_ENTRY) * ATOSE_fat_directory_entry::LONG_FILENAME_CHARACTERS_PER_ENTRY;
current = into;
for (sequence_number = 0; sequence_number < needed; sequence_number++)
	{
	current->LDIR_Ord = needed - sequence_number;
	current->LDIR_Attr = ATOSE_fat_directory_entry::ATTR_LONG_NAME;
	current->LDIR_Type = 0;				// must be 0 if this is part of a long filename
	current->LDIR_FstClusLO = 0;		// must be 0
	current->LDIR_Chksum = checksum;

	from -= 2;		// 2 characters is 4 bytes
	memcpy(current->LDIR_Name3, from, 4);
	from -= 6;		// 6 characters is 12 bytes
	memcpy(current->LDIR_Name2, from, 12);
	from -= 5;		// 5 characters is 10 bytes
	memcpy(current->LDIR_Name1, from, 10);

	current++;
	}

/*
	now we mark the first one as the beginning of the sequence
*/
into->LDIR_Ord |= ATOSE_fat_directory_entry::LAST_LONG_ENTRY;

/*
	Return the length of the sequence
*/
return needed;
}

/*
	ATOSE_FAT::CREATE()
	-------------------
*/
ATOSE_file_control_block *ATOSE_fat::create(ATOSE_file_control_block *fcb, uint8_t *filename)
{
uint32_t last;
uint16_t filename_long[ATOSE_fat_directory_entry::MAX_LONG_FILENAME_LENGTH + 1];
uint8_t filename_8_dot_3[12];
uint64_t first_block;
ATOSE_fat_directory_entry found_file, *shortname_entry;
ATOSE_fat_directory_entry long_filename_pieces[ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER + 1];		// +1 becuase we're going to add the short name to the end of the list
uint32_t sequence_length, filename_len;
uint8_t checksum;

/*
	Make sure the filename isn't too long
*/
filename_len = ASCII_strlen(filename);
if (filename_len > sizeof(fcb->filename) || filename_len > ATOSE_fat_directory_entry::MAX_LONG_FILENAME_LENGTH)
	return NULL;

/*
	Find the directory entry (if there is one), and delete that file
*/
if ((first_block = find_in_directory(&found_file, root_directory_cluster, filename)) != EOF)
	if (unlink(filename) != 0)
		return NULL;

/*
	Create the on-disk filenames, including the UCS-2 long filename and the ASCII short filename
*/
memset(filename_long, 0xFF, sizeof(filename_long));
UTF8_to_ucs2_strcpy(filename_long, filename);
short_name_from_long_name(filename_8_dot_3, filename_long);
filename_8_dot_3[sizeof(filename_8_dot_3) - 1] = '\0';

/*
	Now we must verify that the short filename does not already exit, and if it does then we change it.
*/
for (last = 1; last < MAX_SHORTNAME_RETRIES; last++)
	if ((first_block = find_in_directory(&found_file, root_directory_cluster, filename_8_dot_3)) == EOF)
		break;
	else
		increment_short_filename(filename_8_dot_3, last);

if (last >= MAX_SHORTNAME_RETRIES)
	return 0;			// but it probably took and eternity

/*
	Get the checksum of the short filename
*/
checksum = shortname_checksum(filename_8_dot_3);

/*
	Next we need to generate all the long filename "parts"... who came up with this stuff, its arcane!
*/
sequence_length = long_filename_to_pieces(long_filename_pieces, filename_long, checksum);

/*
	Add the short name to the end of the list
*/
shortname_entry = long_filename_pieces + sequence_length;
memset(shortname_entry, 0, sizeof(*shortname_entry));
sequence_length++;

debug_print_string("\r\nSHORT FILENAME:");
debug_print_string((char *)filename_8_dot_3);
debug_print_this("\r\nLONG FILENAME (sequence length:", sequence_length, ")");
for (uint32_t current = 0; current < sequence_length; current++)
	{
	debug_dump_buffer((uint8_t *)(long_filename_pieces + current), 0, sizeof(*long_filename_pieces));
	debug_print_string("\r\n");
	}
debug_print_string("LONG FILENAME:");
debug_print_string((char *)filename);

return add_to_directory(root_directory_cluster, long_filename_pieces, sequence_length);
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
/*
	At end of file?
*/
if (fcb->current_block == EOF)
	return NULL;

/*
	A zero-length file?
*/
if (fcb->current_block == 0)
	return NULL;

/*
	Find the next block
*/
if ((fcb->current_block = next_cluster_after(fcb->current_block)) == EOF)
	return NULL;

/*
	Read and return it
*/
if (read_cluster(fcb->buffer, fcb->current_block) == 0)
	return fcb->buffer;

return NULL;
}

/*
	ATOSE_FAT::UNLINK()
	-------------------
	return 0 on success
*/
uint32_t ATOSE_fat::unlink(uint8_t *filename)
{
uint32_t sector[bytes_per_sector];
ATOSE_fat_directory_entry stats;
uint64_t cluster, sector_number, fat_offset;
uint32_t fat_copy, error;

/*
	Get the start cluster of the file (and make sure the file exists
*/
if ((cluster = find_in_directory(&stats, root_directory_cluster, filename, DELETE_FILE)) == EOF)
	return 0;

/*
	"Note that a zero-length file - a file that has no data allocated to it - has a first
	cluster number of 0 placed in its directory entry."

	So, if the start cluster is 0 we're done.
*/
if (cluster == 0)
	return 0;

/*
	Else clean up the FAT chain (i.e. put the clusters back on the free list).
*/
do
	{
	/*
		Compute the FAT sector
	*/
	fat_offset = cluster * 4;
	sector_number = reserved_sectors + (fat_offset / bytes_per_sector);

	/*
		read the FAT
	*/
	read_sector(sector, sector_number);

	/*
		find the next cluster in the chain
	*/
	cluster = sector[fat_offset % bytes_per_sector] & 0x0FFFFFFF;
	sector[fat_offset % bytes_per_sector] &= 0xF0000000;		// "The only time that the high 4 bits of FAT32 FAT entries should ever be changed is when the volume is formatted"

	/*
		write the many copies of the FAT
	*/
	for (fat_copy = 0; fat_copy < fats_on_volume; fat_copy++)
		if ((error = write_sector(sector, sector_number + (sectors_per_fat * fat_copy))) != 0)
			return error;
	}
while (cluster < 0x0FFFFFF8);

return 0;
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
				if ((error = write_sector(sector, reserved_sectors + (last_cluster_in_file / entries_per_sector) + (sectors_per_fat * fat_copy))) != 0)
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
return find_in_directory(NULL, root_directory_cluster, fcb->filename, SET_FILE_SIZE, &length_to_become);
}

/*
	ATOSE_FAT::SHORTNAME_CHECKSUM()
	-------------------------------
	"Returns an unsigned byte checksum computed on an unsigned byte
	array. The array must be 11 bytes long and is assumed to contain
	a name stored in the format of a MS-DOS directory entry."
*/
uint8_t ATOSE_fat::shortname_checksum(uint8_t *name)
{
int16_t current;
uint8_t sum;

sum = 0;
for (current = 11; current != 0; current--)
	sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *name++;		//The operation is an unsigned char rotate right

return sum;
}

/*
	ATOSE_FAT::FIND_IN_DIRECTORY()
	------------------------------
	returns the cluster number of the first cluster of "name" in the directory starting at "start_cluster"
	or EOF if it cannot find the name

	To avoid having to write this code over and over again, its been extended to take an action parameter that describes what to
	do when we find the file.  Those actions include : RETURN_FIRST_BLOCK, DELETE_FILE, SET_FILE_FIZE
	in the case of SET_FILE_SIZE, parameter points to a 64-bit integer
*/
uint64_t ATOSE_fat::find_in_directory(ATOSE_fat_directory_entry *stats, uint64_t start_cluster, uint8_t *name, uint32_t action, void *parameter)
{
uint8_t checksum = 0;
uint8_t long_filename[(ATOSE_fat_directory_entry::MAX_LONG_FILENAME_LENGTH + 1) * sizeof(uint16_t)];
uint8_t utf8_long_filename[sizeof(long_filename)];
uint8_t *into;
uint32_t filename_part = 1;
uint8_t cluster[bytes_per_sector * sectors_per_cluster];
uint64_t cluster_id, filesize;
ATOSE_fat_directory_entry *file, *end;

end = (ATOSE_fat_directory_entry *)(cluster + bytes_per_sector * sectors_per_cluster);
for (cluster_id = start_cluster; cluster_id != EOF; cluster_id = next_cluster_after(cluster_id))
	{
	if (read_cluster(cluster, cluster_id) != 0)
		return EOF;
	for (file = (ATOSE_fat_directory_entry *)cluster; file < end; file++)
		{
		/*
			Check for the end of the list of files in this directory
		*/
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			return EOF;			// not found

		/*
			Check if we have a deleted file
		*/
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_FREE)
			{
			filename_part = ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER  + 1;
			continue;
			}

		/*
			Check if we have a long filename
		*/
		if ((file->DIR_Attr & ATOSE_fat_directory_entry::ATTR_LONG_NAME_MASK) == ATOSE_fat_directory_entry::ATTR_LONG_NAME)
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
				checksum = file->LDIR_Chksum;

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
				Check the checksum is consistent across all longname parts
			*/
			if (file->LDIR_Chksum != checksum)
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
				But we don't know which... it might look like a long name, but if the checksum doesn't match its actually a short name
				Either way, there's some translation necessary

				"an 8-bit checksum is computed on the name contained in the short
				directory entry at the time the short and long directory entries are created. All 11
				characters of the name in the short entry are used in the checksum calculation. The
				check sum is placed in every long entry. If any of the check sums in the set of long
				entries do not agree with the computed checksum of the name contained in the
				short entry, then the long entries are treated as orphans"
			*/
			if (shortname_checksum(file->DIR_Name) != checksum)
				filename_part = ATOSE_fat_directory_entry::MAX_LONG_FILENAME_SEQUENCE_NUMNBER  + 1;			// coz we're really a short name

			if (filename_part == 0)
				UCS2_to_utf8_strcpy(utf8_long_filename, (uint16_t *)long_filename, sizeof(utf8_long_filename));
			else
				eight_point_three_to_utf8_strcpy(utf8_long_filename, file->DIR_Name, sizeof(utf8_long_filename));

			/*
				We got the name, now do the comparison (in UTF-8)
			*/
			if (ASCII_strcmp((char *)utf8_long_filename, (char *)name) == 0)
				{
				switch (action)
					{
					case RETURN_FIRST_BLOCK:
debug_print_string("RETURN_FIRST_BLOCK\r\n");
						memcpy(stats, file, sizeof(*stats));
						return ((uint64_t)file->DIR_FstClusHI << 16) | (uint64_t)file->DIR_FstClusLO;
					case DELETE_FILE:
debug_print_string("DELETE_FILE\r\n");
						file->DIR_Name[0] = ATOSE_fat_directory_entry::ENTRY_FREE;
						return write_cluster(cluster, cluster_id);
					case SET_FILE_SIZE:
debug_print_string("SET_FILE_SIZE\r\n");
						/*
							In both FAT and FAT+ we store the low 32 bits in DIR_FileSize
						*/
						filesize = *(uint64_t *)parameter;
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
	ATOSE_FAT::ADD_TO_DIRECTORY()
	-----------------------------
	Search the directory structure for enough free "slots" to fit the given file name and add it as an empty file
*/
uint64_t ATOSE_fat::add_to_directory(uint64_t directory_start_cluster, ATOSE_fat_directory_entry *new_filename, uint32_t parts)
{
uint8_t cluster[bytes_per_sector * sectors_per_cluster];
uint64_t cluster_id;
ATOSE_fat_directory_entry *file, *end;
uint32_t found;

found = 0;
end = (ATOSE_fat_directory_entry *)(cluster + bytes_per_sector * sectors_per_cluster);
for (cluster_id = start_cluster; cluster_id != EOF; cluster_id = next_cluster_after(cluster_id))
	{
	if (read_cluster(cluster, cluster_id) != 0)
		return EOF;

	for (file = (ATOSE_fat_directory_entry *)cluster; file < end; file++)
		{
		if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_END_OF_LIST)
			break;		// We're at the end of the directory
		else if (file->DIR_Name[0] == ATOSE_fat_directory_entry::ENTRY_FREE)
			{
			found++;
			if (found >= parts)
				{
				/*
					We're found a consecutive list of blank entries so we can put the file name in here.
				*/
				}
			}
		else
			found = 0;
		}
	}
return EOF;		// not found
}

