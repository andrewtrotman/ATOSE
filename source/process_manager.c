/*
	PROCESS_MANAGER.C
	-----------------
*/
#include <stdint.h>
#include "atose.h"
#include "process_manager.h"
#include "elf.h"

/*
	ATOSE_PROCESS_MANAGER::ELF_LOAD()
	---------------------------------
	We need to check as much of this as we can because this is an obvious way
	to attack the kernel.

	Return 0 on success all other codes are error codes
*/
uint32_t ATOSE_process_manager::elf_load(const uint8_t *file, uint32_t length)
{
Elf32_Ehdr *header;			// the ELF file header
long header_ok;				// used to check the ELf magic number is correct
uint32_t header_offset;		// location (in the file) of the first header
uint32_t header_size;			// size of each header
uint32_t header_num;			// number of headers
Elf32_Phdr *current_header;	// the current header
long which;					// which header we're currenty looking at

/*
	An ELF file must be at least the length of the header.
*/
if (length < sizeof(Elf32_Ehdr))
	return ELF_BAD_FROM_START;		// the header is broken

header = (Elf32_Ehdr *)file;

/*
	First verify that we have and ELF file
*/
header_ok = 0;
header_ok += (file[EI_MAG0] == 0x7F);
header_ok += (file[EI_MAG1] == 'E');
header_ok += (file[EI_MAG2] == 'L');
header_ok += (file[EI_MAG3] == 'F');

if (header_ok != 4)
	return ELF_BAD_ID;

/*
	Check we have a 32-bit file
*/
if (file[EI_CLASS] != ELFCLASS32)
	return ELF_BAD_BITNESS;

/*
	Check we have a little endian file
*/
if (file[EI_DATA] != ELFDATA2LSB)
	return ELF_BAD_ENDIAN;

/*
	Check we have a file version we understand
*/
if (file[EI_VERSION] != EV_CURRENT)
	return ELF_BAD_VERSION;

/*
	Make sure the header is the right length
*/
if (header->e_ehsize != sizeof(Elf32_Ehdr))
	return ELF_BAD_HEADER;

/*
	Make sure the header is a version we understand
*/
if (header->e_version != EV_CURRENT)
	return ELF_BAD_HEADER_VERSION;

/*
	Now check we're an executable file
*/
if (header->e_type != ET_EXEC)
	return ELF_BAD_TYPE;

/*
	Check we're an ARM executable file
*/
if (header->e_machine != EM_ARM)
	return ELF_BAD_ARCHITECTURE;

/*
	We passed the tests so at this point we think we have an ELF file
	thats ARM 32-bit little endian.  Of course, it might just be chance
	(or a filed designed to fool) that we this far so we're not out of
	hot water yet.

*/
if (header->e_phnum == 0)
	return ELF_BAD_PROGRAM_HEADER;

/*
	Now we move on to the ELF program header
*/
header_offset = header->e_phoff;
header_size = header->e_phentsize;
header_num = header->e_phnum;

/*
	Make sure the program header is the righ size
*/
if (header_size != sizeof(Elf32_Phdr))
	return ELF_BAD_PROGRAM_HEADER_SIZE;

/*
	Make sure the program header is located within the file itself
	this is an obvious attach we can catch
*/
if ((header_offset + header_size * header_num) > length)
	return ELF_BAD_PROGRAM_HEADER_LOCATION;

/*
	Check that we understand what to do with each segment. In all liklihood
	there are only going to three segments (.text, .data and .bss)

	The check is done before the load to make our own unwind semantics easy
*/
current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	/*
		At present we only allow loadable segments (program and data) and
		C++ stack unwind  semantics segments.
	*/
	if (current_header->p_type != PT_ARM_UNWIND && current_header->p_type != PT_LOAD)
		return ELF_BAD_PROGRAM_SEGMENT_TYPE;

	current_header++;
	}

/*
	Now load each executable file segment
*/
current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
#ifdef NEVER
	range[range_current].start = current_header->p_vaddr;
	range[range_current].finish = current_header->p_vaddr + current_header->p_memsz;
	range_current++;

	printf("p_filesz           : %u\n", current_header->p_filesz);

	printf("p_flags            : %u (", current_header->p_flags);
	if (current_header->p_flags & PF_R)
		printf("R");
	if (current_header->p_flags & PF_W)
		printf("W");
	if (current_header->p_flags & PF_X)
		printf("X");
	printf(")\n");
#endif

	/*
		Move on to the next segment
	*/	
	current_header++;
	}

return SUCCESS;
}


/*
	ATOSE_PROCESS_MANAGER::EXECUTE()
	--------------------------------
*/
uint32_t ATOSE_process_manager::execute(const uint8_t *buffer, uint32_t length)
{
uint32_t answer;

ATOSE *os = ATOSE::get_global_entry_point();
os->io << "[CALL TO START A NEW PROCESS...";
answer = elf_load(buffer, length);
os->io << "response:" << answer << "\r\n";

return length;
}
