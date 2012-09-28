/*
	NAND_ONFI_PARAMETERS.H
	----------------------
*/
#ifndef NAND_ONFI_PARAMETERS_H_
#define NAND_ONFI_PARAMETERS_H_

/*
	class ATOSE_NAND_ONFI_PARAMETERS
	--------------------------------
*/
class ATOSE_nand_onfi_parameters
{
public:
	/*
		andable flags for the timing modes
	*/
	enum {NAND_MODE_0 = 0x01, NAND_MODE_1 = 0x02, NAND_MODE_2 = 0x04, NAND_MODE_3 = 0x08, NAND_MODE_4 = 0x10, NAND_MODE_5 = 0x20};

public:
	/*
		REVISION INFORMATION AND FEATURES BLOCK
		---------------------------------------
	*/
	/*
		Parameter page signature [bytes 0-3]
		Byte 0: 4Fh, 'O'
		Byte 1: 4Eh, 'N'
		Byte 2: 46h, 'F'
		Byte 3: 49h, 'I'
	*/
	uint8_t signature[4];

	/*
		Revision number [bytes 4-5]
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
		Features supported [bytes 6-7]
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
	uint16_t features_supported;

	/*
		Optional commands supported [bytes 8-9]
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
	uint16_t optional_commands_supported;


	/*
		ONFI-JEDEC JTG primary advanced command support [byte 10]
		4-7 Reserved (0)
		3 1 = supports Multi-plane Block Erase
		2 1 = supports Multi-plane Copyback Program
		1 1 = supports Multi-plane Page Program
		0 1 = supports Random Data Out
	*/
	uint8_t advanced_command_support;

	uint8_t reserved_bytes_1[1];				// reserved [byte 11]
	uint16_t extended_parameter_page_length;	// Extended parameter page length [byte 12-13]
	uint8_t number_of_parameter_pages;			// Number of duplicate copies of the parameter page [byte 14]
	uint8_t reserved_bytes_2[17];				// reserved [bytes 15-31]

	/*
		MANUFACTURER INFORMATION BLOCK
		------------------------------
	*/
	uint8_t manufacturer[12];				// Device manufacturer (in the case of the FourARM "Micron" [bytes 32-43]
	uint8_t model[20];						// Device model (in the case of the FourARM "MT29F8G08ABABAWP" [bytes 44-63]
	uint8_t jedec_manufacturer_id;			// JEDEC manufacturer ID [byte 64]
	uint16_t date_code;						// Date code [bytes 65-66]
	uint8_t reserved_bytes_3[13];			// Reserved [bytes 67-79]

	/*
		MEMORY ORGANIZATION BLOCK
		-------------------------
	*/
	uint32_t bytes_per_page;				// Number of data bytes per page [bytes 80-83]
	uint16_t spare_bytes_per_page;			// Number of spare bytes per page [bytes 84-85]
	uint8_t obsolete_bytes_1[4];			// Obsolete - Number of data bytes per partial page [bytes 86-89]
	uint8_t obsolete_bytes_2[2];			// Obsolete - Number of spare bytes per partial page [bytes 90-91]
	uint32_t pages_per_block;				// Number of pages per block [bytes 92-95]
	uint32_t blocks_per_lun;				// Number of blocks per logical unit (LUN) [bytes 96-99]
	uint8_t luns;							// Number of logical units (LUNs) [byte 100]

	/*
		Number of address cycles [byte 101]
		4-7 Column address cycles
		0-3 Row address cycles
	*/
	uint8_t address_cycles;
	uint8_t bits_per_cell;					// Number of bits per cell (bits per bit?) [bytes 102]
	uint16_t max_bad_blocks_per_lun;		// Bad blocks maximum per LUN [bytes 103-104]
	uint16_t block_endurance;				// Block endurance [bytes 105-106]
	uint8_t guaranteed_good_blocks_at_start;// Guaranteed valid blocks at beginning of target [byte 107]
	uint16_t guaranteed_block_endurance;	// Block endurance for guaranteed valid blocks [bytes 108-109]
	uint8_t programs_per_page;				// Number of programs per page [byte 110]
	uint8_t obsolete_bytes_3[1];			// Obsolete - Partial programming attributes [byte 111]
	uint8_t ecc_bits;						// Number of bits ECC correctability [bytes 112]

	/*
		Number of plane address bits [byte 113]
		4-7 Reserved (0)
		0-3 Number of plane address bits
	*/
	uint8_t plane_address_bits;

	/*
		Multi-plane operation attributes [bytes 114]
		6-7 Reserved (0)
		5 1 = lower bit XNOR block address restriction
		4 1 = read cache supported
		3 Address restrictions for cache operations
		2 1 = program cache supported
		1 1 = no block address restrictions
		0 Overlapped / concurrent multi-plane support
	*/
	uint8_t multi_plane_operation_attributes;

	/*
		EZ NAND support [byte 115]
		3-7 Reserved (0)
		2 1 = Requires Copyback Adjacency
		1 1 = supports Copyback for other planes & LUNs
	*/
	uint8_t ez_nand_support;
	uint8_t reserved_bytes_4[12];			// Reserved [bytes 116-127]

	/*
		ELECTRICAL PARAMETERS BLOCK
		---------------------------
	*/
	uint8_t io_pin_capacitance_max;			// I/O pin capacitance per chip enable, maximum [byte 128]

	/*
		SDR timing mode support [byte  129-130]
		6-15 Reserved (0)
		5 1 = supports timing mode 5
		4 1 = supports timing mode 4
		3 1 = supports timing mode 3
		2 1 = supports timing mode 2
		1 1 = supports timing mode 1
		0 1 = supports timing mode 0, shall be 1
	*/
	uint16_t sdr_timing_mode_support;
	uint8_t obsolete_bytes_4[2];			// Obsolete - SDR program cache timing mode support [bytes 131-132]
	uint16_t t_prog;						// tPROG Maximum PROGRAM PAGE time (us) [bytes 133-134]
	uint16_t t_bers;						// tBERS Maximum BLOCK ERASE time (us) [bytes 135-136]
	uint16_t t_r;							// tR Maximum PAGE READ time (us) [bytes 137-138]
	uint16_t t_ccs;							// tCCS Minimum change column setup time (ns) [bytes 139-140]

	/*
		NV-DDR timing mode support [byte 141]
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
		NV-DDR2 timing mode support [byte 142]
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
		NV-DDR / NV-DDR2 features [bytes 143]
		3-7 Reserved (0)
		2 1 = device supports CLK stopped for data input
		1 1 = typical capacitance values present
		0 tCAD value to use
	*/
	uint8_t ddr_features;
	uint16_t clk_pin_capacitance_typical;	// CLK input pin capacitance, typical [bytes 144-145]
	uint16_t io_pin_capacitance_typical;	// I/O pin capacitance, typical [bytes 146-147]
	uint16_t input_pin_capacitance_typical;	// Input capacitance, typical [bytes 148-149]
	uint8_t input_pin_capacitance_max;		// Input pin capacitnace, maximum [byte 150]

	/*
		Driver strength support [byte 151]
		3-7 Reserved (0)
		2 1 = supports 18 Ohm drive strength
		1 1 = supports 25 Ohm drive strength
		0 1 = supports driver strength settings
	*/
	uint8_t driver_strength_support;
	uint16_t t_r_maximum;					// tR Maximum multi-plane page read time (us) [bytes 152-153]
	uint16_t t_adl;							// tADL Program page register clear enhancement tADL value (ns) [bytes 154-155]
	uint16_t t_r_typical;					// tR Typical page read time for EZ NAND (us) [bytes 156-157]

	/*
		NV-DDR2 features [byte 158]
		5-7 Reserved (0)
		4 1 = supports differential signaling for DQS
		3 1 = supports differential signaling for RE_n
		2 1 = supports ODT value of 30 Ohms
		1 1 = supports matrix termination ODT
		0 1 = supports self-termination ODT
	*/
	uint8_t ddr2_features;

	/*
		NV-DDR2 warmup cycles [byte 159]
		4-7 Data Input warmup cycles support
		0-3 Data Output warmup cycles support
	*/
	uint8_t ddr2_warmup_cycles;

	uint8_t reserved_bytes_5[4];			// Reserved [bytes 160-163]

	/*
		VENDOR BLOCK
		------------
	*/
	uint16_t vendor_specific_revision;		// Vendor specific Revision number [bytes 164-165]
	uint8_t reserved_bytes_6[88];			// Reserved [bytes 166-253]

	/*
		Integrity CRC [bytes 254-255]

		The Integrity CRC (Cyclic Redundancy Check) field is used to
		verify that the contents of the parameter page were transferred
		correctly to the host. The CRC of the parameter page is a word
		(16-bit) field. The CRC calculation covers all of data between
		byte 0 and byte 253 of the parameter page inclusive.

		The CRC shall be calculated on byte (8-bit) quantities starting
		with byte 0 in the parameter page. The bits in the 8-bit quantity
		are processed from the most significant bit (bit 7) to the least
		significant bit (bit 0).

		The CRC shall be calculated using the following 16-bit generator
		polynomial:

		G(X) = X16 + X15 + X2 + 1

		This polynomial in hex may be represented as 8005h.

		The CRC value shall be initialized with a value of 4F4Eh before
		the calculation begins. There is no XOR applied to the final CRC
		value after it is calculated. There is no reversal of the data
		bytes or the CRC calculated value.
*/

	uint16_t crc;

	/*
		REDUNDANT PARAMETER PAGES
		-------------------------
	*/
	/*
		Several duplicates of the parameter page follow the first.  The ONFI spec lists 2 extra copies
		as mandatory and any other copies are optional.  We'll ignore any copyies other than the first
		for the purpose of defining this structure as we can re-use the same structure if we are forced
		to read several copies.
	*/
public:
	static uint16_t compute_crc(uint8_t *buffer, uint32_t length);

} __attribute__((packed));

/*
	ATOSE_NAND_ONFI_PARAMETERS::COMPUTE_CRC()
	-----------------------------------------
*/
inline uint16_t ATOSE_nand_onfi_parameters::compute_crc(uint8_t *buffer, uint32_t length)
{
uint32_t bit;
uint32_t byte;
uint16_t crc = 0x4F4E;

for (byte = 0; byte < length; byte++)
	{
	crc ^= *buffer << 8;
	buffer++;

	for (bit = 0; bit < 8; bit++)
		crc = (crc << 1) ^ ((crc & 0x8000) ? 0x8005 : 0);
	}

return crc;
}

#endif /* NAND_ONFI_PARAMETERS_H_ */
