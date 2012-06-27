/*
	PIC_IMX233.C
	------------
*/
#include <stdio.h>
#include "pic_imx233.h"
#include "device_driver.h"

/*
	ATOSE_PIC_IMX233::INIT()
	------------------------
*/
void ATOSE_pic_imx233::init(void)
{
uint32_t current;

/*
	Initialise the interrupt vector table to all zero
*/
for (current = 0; current < HW_ICOLL_INTERRUPTn_COUNT; current++)
	interrupt_vector_table[current] = NULL;

/*
	Give the interrupt table to the ICOLL interrupt controller
*/
HW_ICOLL_VBASE_WR((uint32_t)&interrupt_vector_table);

/*
	Tell the ICOLL interrupt controller to be in RSE mode ("read" to signal entry to ISR) and to enable IRQ
*/
HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_ARM_RSE_MODE | BM_ICOLL_CTRL_IRQ_FINAL_ENABLE);
}

/*
	ATOSE_PIC_IMX233::ENABLE()
	--------------------------
*/
void ATOSE_pic_imx233::enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary)
{
/*
	Put the interrupt address into the interrupt vector table
*/
interrupt_vector_table[primary] = driver;

/*
	Enable the interrupt at level 0
*/
HW_ICOLL_INTERRUPTn_SET(primary, BM_ICOLL_INTERRUPTn_ENABLE);
}

/*
	ATOSE_PIC_IMX233::ACKNOWLEDGE()
	-------------------------------
*/
void ATOSE_pic_imx233::acknowledge(void)
{
}