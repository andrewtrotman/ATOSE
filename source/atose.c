/*
	ATOSE.C
	-------
*/
#include "atose.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbh.h"		// DELETE
#include "nand_onfi_parameters.h" // DELETE THIS LINE

extern ATOSE *ATOSE_addr;

/*
	ATOSE::ATOSE()
	--------------
*/
ATOSE::ATOSE()
{
/*
	First things first... put a pointer to me at the end of the interrupt space so that
	we can at ant time get a pointer to this object (by calling ATOSE::get_global_entry_point()
*/
ATOSE_addr = this;

/*
	Initialise each of the essential core objects
*/
stack.init();
cpu.init();
pic.init();
io.init();
timer.init();
heap.init();

#ifdef IMX233
	disk.init();
#else
	keyboard.init((unsigned char *)0x10006000);
	mouse.init((unsigned char *)0x10007000);
#endif

/*
	Now join them together to form a working system
*/
#ifdef IMX233
	cpu.enable_IRQ();

//	pic.enable(&timer, VECTOR_IRQ_RTC_1MSEC);
//	timer.enable();

	pic.enable(&io, VECTOR_IRQ_DEBUG_UART);
	io.enable();

	io << "\r\n\r\nATOSE\r\nIO enabled\r\n";
	/*
		The NAND interface can cause three possible interrupts!
	*/
	pic.enable(&disk, VECTOR_IRQ_GPMI);
	pic.enable(&disk, VECTOR_IRQ_GPMI_DMA);
	pic.enable(&disk, VECTOR_IRQ_BCH);
	disk.enable();

	io << "FLASH enabled\r\n";

#ifdef NEVER
				{
				/*
					Remove this block of code.  Move the reset() into the disk.enable().
				*/
				io.hex();
				io << "DISK enabled\r\n";
				io << "APBH Semaphore value:" << (uint32_t)(HW_APBH_CHn_SEMA(4).B.PHORE) << "\r\n";
				io << "DISK status\r\n";

				uint32_t status = disk.status();

				static __attribute__((aligned(0x04))) uint8_t buffer[4096 + 224];
				for (int x = 0; x < sizeof(buffer); x++)
					buffer[x] = 0;

				ATOSE_nand_onfi_parameters *params = (ATOSE_nand_onfi_parameters *)buffer;
				
				io << "\r\nNAND get parameter block \r\n";

				uint32_t trials = disk.get_parameter_block((ATOSE_nand_onfi_parameters *)buffer);
				
				io << "\r\nsuccess after " << trials << " failures\r\n";
				

				int x = 0;
				for (int row = 0; row < 0x0F; row++)
					{
					int old_x = x;
					for (int column = 0; column < 0xF; column++)
						io << (uint32_t)buffer[x++] << " ";

					for (int column = 0; column < 0xF; column++)
						{
						io << (char)((buffer[old_x] >= ' ' && buffer[old_x] <= 'Z') ? buffer[old_x] : ' ');
						old_x++;
						}
					io << "\r\n";
					}

				io.decimal();
				io << "Number of data bytes per page  :" << (uint32_t)(params->bytes_per_page) << "\r\n";
				io << "Number of spare bytes per page :" << (uint32_t)(params->spare_bytes_per_page) << "\r\n";
				io << "Number of pages per block      :" << (uint32_t)(params->pages_per_block) << "\r\n";
				io << "Number of blocks per lun       :" << (uint32_t)(params->blocks_per_lun) << "\r\n";
				io << "Number of luns                 :" << (uint32_t)(params->luns) << "\r\n";
				io << "ECC bits                       :" << (uint32_t)(params->ecc_bits) <<  "\r\n";
				io << "SDR Timing mode support        :" << (uint32_t)(params->sdr_timing_mode_support) <<  "\r\n";

			/*
				for (int x = 0; x < sizeof(buffer); x++)
					buffer[x] = 0;

				io.hex();
				io << "DISK read\r\n";

				uint32_t fixed_bits = disk.read_sector(buffer, 123);
				for (int x = 0; x < 4096; x++)
					io << (uint32_t)buffer[x] << " ";

				io.decimal();
				io << "\r\nFIXED: " << fixed_bits << " bits\r\n";
			*/
				io << "\r\nWRITE\r\n";
				for (int x = 0; x < 4096; x++)
					buffer[x] = (uint8_t)(x + 102);
				uint32_t ok = disk.write_sector(buffer, 102);

				io << "\r\nWRITE STAUTS:" << ok << "\r\n";

				io << "\r\nREAD\r\n";
				uint32_t fixed_bits = disk.read_sector(buffer, 102);
				for (int x = 0; x < 4096; x++)
					io << (uint32_t)buffer[x] << " ";
				io.decimal();
				io << "\r\nFIXED: " << fixed_bits << " bits\r\n";


				}
#endif

#else
	cpu.enable_IRQ();
	pic.enable(&timer, 0x04);
	timer.enable();
	pic.enable(&io, 0x0C);
	io.enable();
	pic.enable(&keyboard, 0x1F, 0x03);
	keyboard.enable();
	pic.enable(&mouse, 0x1F, 0x04);
	mouse.enable();
#endif
}

/*
	ATOSE::GET_GLOBAL_ENTRY_POINT()
	-------------------------------
*/
ATOSE *ATOSE::get_global_entry_point(void)
{
return ATOSE_addr;
}
