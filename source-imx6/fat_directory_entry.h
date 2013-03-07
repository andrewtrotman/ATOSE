/*
	FAT_DIRECTORY_ENTRY.H
	---------------------
*/
#ifndef FAT_DIRECTORY_ENTRY_H_
#define FAT_DIRECTORY_ENTRY_H_

/*
	class ATOSE_FAT_DIRECTORY_ENTRY
	-------------------------------
	see "Microsoft Extensible Firmware Initiative FAT32 File System Specification FAT: General Overview of On- Disk Format
	Version 1.03, December 6, 2000 Microsoft Corporation" pages 26.

*/
class ATOSE_fat_directory_entry
{
public:
	/*
		"Long names are limited to 255 characters, not including the trailing NUL. The total
		path length of a long name cannot exceed 260 characters, including the trailing
		NUL"

		According to Wikipedia, these are encoded in UCS-2.
	*/
	static const uint32_t MAX_LONG_FILENAME_LENGTH = 255;

	/*
		"First, every member of a set of long entries is uniquely numbered and the last
		member of the set is or'd with a flag indicating that it is, in fact, the last member of
		the set. The LDIR_Ord field is used to make this determination. The first member
		of a set has an LDIR_Ord value of one. The nth long member of the set has a value
		of (n OR LAST_LONG_ENTRY)."
	*/
	static const uint32_t LAST_LONG_ENTRY = 0x40;

	/*
		The number of bytes of filename that are stored in a long filename entry (13 characters = 26 bytes)
	*/
	static const uint32_t LAST_LONG_BYTES_PER_SHORT_NAME = 26;

	/*
		Each long filename entry contains 13 characters (26 bytes) and the maximum length of a
		long filename is 255 bytes so we get...
	*/
	static const uint32_t MAX_LONG_FILENAME_SEQUENCE_NUMNBER = (MAX_LONG_FILENAME_LENGTH / 13) + 1;

	/*
		"If DIR_Name [0] == 0xE5, then the directory entry is free (there is no file or directory name in this entry"
	*/
	static const uint8_t ENTRY_FREE = 0xE5;

	/*
		"If DIR_Name [0] == 0x05, then the actual file name character for this byte is 0xE5.
		0xE5 is actually a valid KANJI lead byte value for the character set used in Japan."
	*/
	static const uint8_t ENTRY_USE_0xE5 = 0x05;

	/*
		"If DIR_Name [0] == 0x00, then the directory entry is free (same as for 0xE5), and
		there are no allocated directory entries after this one (all of the DIR_Name [0]
		bytes in all of the entries after this one are also set to 0)."
	*/
	static const uint8_t ENTRY_END_OF_LIST = 0x00;

	/*
		DIR_Attr bit flags
	*/
	static const uint8_t ATTR_READ_ONLY = 0x01;
	static const uint8_t ATTR_HIDDEN = 0x02;
	static const uint8_t ATTR_SYSTEM = 0x04;
	static const uint8_t ATTR_VOLUME_ID = 0x08;
	static const uint8_t ATTR_DIRECTORY = 0x10;
	static const uint8_t ATTR_ARCHIVE = 0x20;
	static const uint8_t ATTR_LONG_NAME = (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID);

	/*
		"Date Format. A FAT directory entry date stamp is a 16- bit field that is basically a
		date relative to the MS-DOS epoch of 01/01 / 19 80. Here is the format (bit 0 is the
		LSB of the 16- bit word, bit 15 is the MSB of the 16- bit word):
			Bits 0–4: Day of month, valid value range 1- 31 inclusive.
			Bits 5–8: Month of year, 1 = January, valid value range 1–12 inclusive.
			Bits 9–15: Count of years from 1980, valid value range 0–127 inclusive (1980–2107)."

		"Time Format. A FAT directory entry time stamp is a 16- bit field that has a
		granulari ty of 2 seconds. Here is the format (bit 0 is the LSB of the 16- bit word, bit
		15 is the MSB of the 16- bit word).
			Bits 0–4: 2- second count, valid value range 0–29 inclusive (0 – 58 seconds).
			Bits 5–10: Minutes, valid value range 0–59 inclusive.
			Bits 11–15: Hours, valid value range 0–23 inclusive.
		The valid time range is from Midnight 00:00:00 to 23:59:58."
	*/
public:
	enum {SHORT_FILENAME, ORPHAN_FILENAME, LONG_FILENAME};

public:
	union
		{
		struct
			{
			uint8_t DIR_Name[8];      // 0
			uint8_t DIR_Extension[3]; // 8
			uint8_t DIR_Attr;         // 11
			uint8_t DIR_NTRes;        // 12
			uint8_t DIR_CrtTimeTenth; // 13
			uint16_t DIR_CrtTime;     // 14
			uint16_t DIR_CrtDate;     // 16
			uint16_t DIR_LstAccDate;  // 18
			uint16_t DIR_FstClusHI;   // 20
			uint16_t DIR_WrtTime;     // 22
			uint16_t DIR_WrtDate;     // 24
			uint16_t DIR_FstClusLO;   // 26
			uint32_t DIR_FileSize;    // 28
			} __attribute__ ((packed));
		struct
			{
			uint8_t LDIR_Ord;				// 0
			uint8_t LDIR_Name1[10];		// 1
			uint8_t LDIR_Attr;			// 11		// same location as DIR_Attr "must be ATTR_LONG_NAME"
			uint8_t LDIR_Type;			// 12
			uint8_t LDIR_Chksum;			// 13
			uint8_t LDIR_Name2[12];		// 14
			uint16_t LDIR_FstClusLO;	// 26		// same location as DIR_FstClusLO
			uint8_t LDIR_Name3[4];		// 28
			} __attribute__ ((packed));
		} __attribute__ ((packed));
} __attribute__ ((packed));


#endif

