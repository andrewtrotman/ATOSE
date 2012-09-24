/*
	NAND_VERIFY.H
	-------------
*/
#ifndef NAND_VERIFY_H_
#define NAND_VERIFY_H_

#include "nand.h"

/*
	class ATOSE_NAND_VERIFY
	-----------------------
*/
template <class BASE>
class ATOSE_nand_verify : public BASE
{
public:
	/*
		The interface to the NAND chip
	*/
	virtual uint32_t read_sector(uint8_t *destination, uint64_t sector);
	virtual uint32_t write_sector(uint8_t *buffer, uint64_t sector);
	virtual uint32_t erase_block(uint64_t sector);

public:
	ATOSE_nand_verify() : BASE() {}
} ;



/*
	ATOSE_NAND_VERIFY::READ_SECTOR()
	--------------------------------
*/
template <class BASE>
uint32_t ATOSE_nand_verify<BASE>::read_sector(uint8_t *destination, uint64_t sector)
{
uint32_t result;

/*
	Do the read then if we think it worked then check the status register to make sure it worked.
*/
if ((result = BASE::read_sector(destination, sector)) == 0)
	if (BASE::status() & (ATOSE_nand::FAIL | ATOSE_nand::FAILC) != 0)
		return 0x01;

return result;
}

/*
	ATOSE_NAND_VERIFY::WRITE_SECTOR()
	---------------------------------
*/
template <class BASE>
uint32_t ATOSE_nand_verify<BASE>::write_sector(uint8_t *data, uint64_t sector)
{
uint32_t result;

/*
	Do the write then if we think it worked then check the status register to make sure it worked.
*/
if ((result = BASE::write_sector(data, sector)) == 0)
	if (BASE::status() & (ATOSE_nand::FAIL | ATOSE_nand::FAILC) != 0)
		return 0x01;

return result;
}

/*
	ATOSE_NAND_VERIFY::ERASE_BLOCK()
	--------------------------------
*/
template <class BASE>
uint32_t ATOSE_nand_verify<BASE>::erase_block(uint64_t sector)
{
uint32_t result;

if ((result = BASE::erase_block(sector)) == 0)
	if (BASE::status() & (ATOSE_nand::FAIL | ATOSE_nand::FAILC) != 0)
		return 0x01;

return result;
}



#endif /* NAND_VERIFY_H_ */
