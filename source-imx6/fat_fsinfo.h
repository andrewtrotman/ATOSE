/*
	FAT_FSINFO.H
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef FAT_FSINFO_H_
#define FAT_FSINFO_H_

/*
	class ATOSE_FAT_FSINFO
	----------------------
*/
class ATOSE_fat_fsinfo
{
public:
	static const uint32_t LEAD_SIGNATURE = 0x41615252;
	static const uint32_t STRUCT_SIGNATURE = 0x61417272;
	static const uint32_t TAIL_SIGNATURE = 0xAA550000;

public:
	uint32_t FSI_LeadSig;
	uint8_t  FSI_Reserved[480];
	uint32_t FSI_StrucSig;
	uint32_t FSI_Free_Count;						// if 0xFFFFFFFF, it is known to be unknown
	uint32_t FSI_Nxt_Free;							// if 0xFFFFFFFF, then it is unknown
	uint8_t 	FSI_Reserved2[12];
	uint32_t FSI_TrailSig;
} ;
#endif
