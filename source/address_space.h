/*
	ADDRESS_SPACE.H
	---------------
*/
#ifndef ADDRESS_SPACE_H_
#define ADDRESS_SPACE_H_

class ATOSE_mmu;

/*
	class ATOSE_ADDRESS_SPACE
	-------------------------
*/
class ATOSE_address_space
{
private:
	ATOSE_mmu *mmu;
	ATOSE_mmu_page_list page_list;
	uint32_t *page_table;
	uint32_t process_id;

protected:
	uint32_t add_page(void *virtual_address, ATOSE_mmu_page *page, uint32_t type);

public:
	ATOSE_address_space(ATOSE_mmu *mmu, uint32_t process_id) { this->mmu = mmu; this->process_id = process_id; }
	ATOSE_address_space *create(void);
	uint32_t destroy(void);
} ;


#endif /* ADDRESS_SPACE_H_ */
