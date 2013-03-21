/*
	FAT_BOOT_SECTOR.H
	-----------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef FAT_BOOT_SECTOR_H_
#define FAT_BOOT_SECTOR_H_

#include "msb_lsb.h"

/*
	class ATOSE_FAT_BOOT_SECTOR
	---------------------------
	see "Microsoft Extensible Firmware Initiative FAT32 File System Specification FAT: General Overview of On- Disk Format
	Version 1.03, December 6, 2000 Microsoft Corporation" pages 9-14.
*/
class ATOSE_fat_boot_sector
{
union
	{
	uint8_t buffer[512];
	struct
		{
		uint8_t BS_jmpBoot[3];    //  0  (offset from start of structure)
		uint8_t BS_OEMName[8];    //  3
		ATOSE_lsb_uint16_t BPB_BytsPerSec; // 11
		uint8_t BPB_SecPerClus;   // 13
		uint16_t BPB_RsvdSecCnt;  // 14
		uint8_t BPB_NumFATs;      // 16
		ATOSE_lsb_uint16_t BPB_RootEntCnt; // 17
		ATOSE_lsb_uint16_t BPB_TotSec16;   // 19
		uint8_t BPB_Media;        // 21
		uint16_t BPB_FATSz16;     // 22
		uint16_t BPB_SecPerTrk;   // 24
		uint16_t BPB_NumHeads;    // 26
		uint32_t BPB_HiddSec;     // 28
		uint32_t BPB_TotSec32;    // 32
		uint32_t BPB_FATSz32;     // 36
		uint16_t BPB_ExtFlags;    // 40
		uint16_t BPB_FSVer;       // 42
		uint32_t BPB_RootClus;    // 44		// location of the root directory
		uint16_t BPB_FSInfo;      // 48
		uint16_t BPB_BkBootSec;   // 50
		uint8_t BPB_Reserved[12]; // 52
		uint8_t BS_DrvNum;        // 64
		uint8_t BS_Reserved1;     // 65
		uint8_t BS_BootSig;       // 66
		ATOSE_lsb_uint32_t BS_VolID;       // 67
		uint8_t BS_VolLab[11];    // 71
		char BS_FilSysType[8]; // 82
		} __attribute__ ((packed));
	struct
		{
		uint8_t bytes_stream[510];
		uint8_t should_be_0x55;
		uint8_t should_be_0xAA;
		} __attribute__ ((packed));
	} __attribute__ ((packed));
} __attribute__ ((packed));

#endif
