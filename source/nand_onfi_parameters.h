/*
	NAND_ONFI_PARAMETERS.H
	----------------------
*/
#ifndef NAND_ONFI_PARAMETERS_H_
#define NAND_ONFI_PARAMETERS_H_

/*
	class ATOSE_NAMD_ONFI_PARAMETER
	-------------------------------
*/
class ATOSE_namd_onfi_parameter
{
public:
	/*
		REVISION INFORMATION AND FEATURES BLOCK
		---------------------------------------
	*/
	/*
	Parameter page signature
	Byte 0: 4Fh, 'O'
	Byte 1: 4Eh, 'N'
	Byte 2: 46h, 'F'
	Byte 3: 49h, 'I'
	*/
	uint8_t signature[4];

	/*
		Revision number
		7-15 Reserved (0)
		6 1 = supports ONFI version 3.0
		5 1 = supports ONFI version 2.3
		4 1 = supports ONFI version 2.2
		3 1 = supports ONFI version 2.1
		2 1 = supports ONFI version 2.0
		1 1 = supports ONFI version 1.0
		0 Reserved (0)
	*/
	uint16_t revision_number;

	/*
		Features supported
		13-15 Reserved (0)
		12 1 = supports external Vpp
		11 1 = supports Volume addressing
		10 1 = supports NV-DDR2
		9 1 = supports EZ NAND
		8 1 = supports program page register clear enhancement
		7 1 = supports extended parameter page
		6 1 = supports multi-plane read operations
		5 1 = supports NV-DDR
		4 1 = supports odd to even page Copyback
		3 1 = supports multi-plane program and erase operations
		2 1 = supports non-sequential page programming
		1 1 = supports multiple LUN operations
		0 1 = supports 16-bit data bus width
	*/
	uint16_t features;

	/*
		Optional commands supported
		12-15 Reserved (0)
		11 1 = supports ODT Configure
		10 1 = supports Volume Select
		9 1 = supports Reset LUN
		8 1 = supports Small Data Move
		7 1 = supports Change Row Address
		6 1 = supports Change Read Column Enhanced
		5 1 = supports Read Unique ID
		4 1 = supports Copyback
		3 1 = supports Read Status Enhanced
		2 1 = supports Get Features and Set Features
		1 1 = supports Read Cache commands
		0 1 = supports Page Cache Program command
	*/
	uint16_t optional_command_support;


	/*
		ONFI-JEDEC JTG primary advanced command support
		4-7 Reserved (0)
		3 1 = supports Multi-plane Block Erase
		2 1 = supports Multi-plane Copyback Program
		1 1 = supports Multi-plane Page Program
		0 1 = supports Random Data Out
	*/
	uint8_t advanced_command_support

	uint8_t reserved_bytes_1[1];				// reserved
	uint8_t extended_parameter_page_length		// Extended parameter page length
	uint8_t number_of_parameter_pages;			// Number of duplicate copies of the parameter page
	uint8_t reserved_bytes_2[17];				// reserved

	/*
		MANUFACTURER INFORMATION BLOCK
		------------------------------
	*/
	uint8_t manufacturer[12];				// Device manufacturer (in the case of the FourARM "Micron"
	uint8_t model[20];						// Device model (in the case of the FourARM "MT29F8G08ABABAWP"
	uint8_t jedec_manufacturer_id;			// JEDEC manufacturer ID
	uint16_t date_code;						// Date code
	uint8_t reserved_bytes_3[13];			// Reserved

	/*
		MEMORY ORGANIZATION BLOCK
		-------------------------
	*/
	uint32_t byte_per_page;					// Number of data bytes per page
	uint16_t spare_bytes_per_page;			// Number of spare bytes per page
	uint32_t data_bytes_per_partial_page;	// Obsolete - Number of data bytes per partial page
	uint16_t spare_bytes_per_partial_page;	// Obsolete - Number of spare bytes per partial page
	uint32_t pages_per_block;				// Number of pages per block
	uint32_t blocks_per_lun;				// Number of blocks per logical unit (LUN)
	uint8_t number_of_luns;					// Number of logical units (LUNs)

	/*
		Number of address cycles
		4-7 Column address cycles
		0-3 Row address cycles
	*/
	uint8_t address_cycles;

	uint8_t bits_per_cell;					// Number of bits per cell (bits per bit?)
	uint16_t max_bad_blocks_per_lun;		// Bad blocks maximum per LUN
	uint16_t block_endurance;				// Block endurance
	uint8_t guaranteed_good_blocks;			// Guaranteed valid blocks at beginning of target
	uint16_t guaranteed_block_endurance;	// Block endurance for guaranteed valid blocks
	uint8_t programs_per_page;				// Number of programs per page

	uint8_t partial_page_attributes;		// Obsolete - Partial programming attributes
	uint8_t ecc_bits;						// Number of bits ECC correctability

	/*
		Number of plane address bits
		4-7 Reserved (0)
		0-3 Number of plane address bits
	*/
	uint8_t interleaved_bits;

	/*
		Multi-plane operation attributes
		6-7 Reserved (0)
		5 1 = lower bit XNOR block address restriction
		4 1 = read cache supported
		3 Address restrictions for cache operations
		2 1 = program cache supported
		1 1 = no block address restrictions
		0 Overlapped / concurrent multi-plane support
	*/
	uint8_t interleaved_ops;

	/*
		EZ NAND support
		3-7 Reserved (0)
		2 1 = Requires Copyback Adjacency
		1 1 = supports Copyback for other planes & LUNs
	*/
	uint8_t ez_nand_support;
	uint8_t reserved_bytes_4[12];			// Reserved

	/*
		ELECTRICAL PARAMETERS BLOCK
		---------------------------
	*/
	uint8_t io_pin_capacitance_max;			// I/O pin capacitance per chip enable, maximum

	/*
		SDR timing mode support
		6-15 Reserved (0)
		5 1 = supports timing mode 5
		4 1 = supports timing mode 4
		3 1 = supports timing mode 3
		2 1 = supports timing mode 2
		1 1 = supports timing mode 1
		0 1 = supports timing mode 0, shall be 1
	*/
	uint16_t sdr_timing_mode;
	uint16_t program_cache_timing_mode;		// Obsolete - SDR program cache timing mode support
	uint16_t t_prog;						// tPROG Maximum PROGRAM PAGE time (us)
	uint16_t t_bers;						// tBERS Maximum BLOCK ERASE time (us)
	uint16_t t_r;							// tR Maximum PAGE READ time (us)
	uint16_t t_ccs;							// tCCS Minimum change column setup time (ns)

	/*
		NV-DDR timing mode support
		6-7 Reserved (0)
		5 1 = supports timing mode 5
		4 1 = supports timing mode 4
		3 1 = supports timing mode 3
		2 1 = supports timing mode 2
		1 1 = supports timing mode 1
		0 1 = supports timing mode 0
	*/
	uint8_t ddr_timing_mode_support;

	/*
		NV-DDR2 timing mode support
		7 1 = supports timing mode 7
		6 1 = supports timing mode 6
		5 1 = supports timing mode 5
		4 1 = supports timing mode 4
		3 1 = supports timing mode 3
		2 1 = supports timing mode 2
		1 1 = supports timing mode 1
		0 1 = supports timing mode 0
	*/
	uint8_t ddr2_timing_mode_support;

	/*
		NV-DDR / NV-DDR2 features
		3-7 Reserved (0)
		2 1 = device supports CLK stopped for data input
		1 1 = typical capacitance values present
		0 tCAD value to use
	*/
	uint16_t source_synchronous_features;

	uint16_t clk_pin_capacitance_typ;		// CLK input pin capacitance, typical
	uint16_t io_pin_capacitance_typ;		// I/O pin capacitance, typical
	uint16_t input_pin_capacitance_typ;		// Input capacitance, typical
	uint8_t input_pin_capacitance_max;		// Input pin capacitnace, maximum

	/*
		Driver strength support
		3-7 Reserved (0)
		2 1 = supports 18 Ohm drive strength
		1 1 = supports 25 Ohm drive strength
		0 1 = supports driver strength settings
	*/
	uint8_t driver_strength_support;
	uint16_t t_int_r;						// tR maximum interleaved (multi-plane) page read time (us)
	uint16_t t_ald;							// tADL Program page register clear enhancement tADL value (ns)
	uint16_t t_r_typical;					// tR Typical page read time for EZ NAND (us)
	/*
		NV-DDR2 features
		5-7 Reserved (0)
		4 1 = supports differential signaling for DQS
		3 1 = supports differential signaling for RE_n
		2 1 = supports ODT value of 30 Ohms
		1 1 = supports matrix termination ODT
		0 1 = supports self-termination ODT
	*/
	uint8_t ddr2_features;

	/*
		NV-DDR2 warmup cycles
		4-7 Data Input warmup cycles support
		0-3 Data Output warmup cycles support
	*/
	uint8_t ddr2_warmup_cycles;

	uint8_t reserved_bytes_5[4];			// Reserved

	/*
		VENDOR BLOCK
		------------
	*/
	uint16_t vendor_specific_revision_number;// Vendor specific Revision number
	uint8_t reserved_bytes_6[88];
	uint16_t crc;							// Integrity CRC

	/*
		REDUNDANT PARAMETER PAGES
		-------------------------
	*/
} __attribute__((packed));


#endif /* NAND_ONFI_PARAMETERS_H_ */
