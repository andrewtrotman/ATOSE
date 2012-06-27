
/*
	PIC_IMX233.H
	------------
*/
#ifndef PIC_IMX233_H_
#define PIC_IMX233_H_

#include <stdint.h>
#include "pic.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsicoll.h"


/*
	class ATOSE_PIC_IMX233
	----------------------
*/
class ATOSE_pic_imx233 : public ATOSE_pic
{
private:
	ATOSE_device_driver *interrupt_vector_table[HW_ICOLL_INTERRUPTn_COUNT];

public:
	ATOSE_pic_imx233() {}
	virtual void init(void);

	virtual void enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary = NO_SECONDARY);
	virtual void acknowledge(void);
} ;

#endif /* PIC_IMX233_H_ */
