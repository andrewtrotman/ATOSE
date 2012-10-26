/*
	PROCESS_MANAGER.C
	-----------------
*/
#include <stdint.h>
#include "atose.h"
#include "ascii_str.h"
#include "process_manager.h"
#include "elf.h"

/*
	ATOSE_PROCESS_MANAGER::ELF_LOAD()
	---------------------------------
	We need to check as much of this as we can because this is an obvious way
	to attack the kernel.

	Return 0 on success all other codes are error codes
*/
uint32_t ATOSE_process_manager::elf_load(ATOSE_process *process, const uint8_t *file, uint32_t length)
{
Elf32_Ehdr *header;			// the ELF file header
uint32_t header_ok;			// used to check the ELf magic number is correct
uint32_t header_offset;		// location (in the file) of the first header
uint32_t header_size;			// size of each header
uint32_t header_num;			// number of headers
Elf32_Phdr *current_header;	// the current header
uint32_t which;				// which header we're currenty looking at
uint32_t permissions;			// permissions on pages in the address space (READ / WRITE / EXECUTE / etc.)
uint8_t *into;					// pointer to a segment returned by the address space

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

	/*
		Make sure that in all cases the size in the ELF file is no larger than the amount
		of memory needed to store it.
	*/
	if (current_header->p_filesz > current_header->p_memsz)
		return ELF_BAD_PROGRAM_SEGMENT_SIZE;

	/*
		Make sure we don't read past end of file
	*/
	if (current_header->p_offset + current_header->p_filesz > length)
		return ELF_BAD_PROGRAM_SEGMENT_LOCATION;

	current_header++;
	}

/*
	We passed all the initial tests so we create the address space
*/
if (process->address_space.create() == 0)
	return ELF_BAD_ADDRESS_SPACE_FAIL;

/*
	Add the necessary pages into it
*/
current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	/*
		Get the page permissions (this code is somewhat redundant because
		we've set up the permission flags in address_space->add() to
		match those of the ELF so that we can use current_header->p_flags
		without the translation below.  In the mean time we leave this
		code in here.
	*/
	permissions = ATOSE_address_space::NONE;
	if (current_header->p_flags & PF_R)
		permissions += ATOSE_address_space::READ;
	if (current_header->p_flags & PF_W)
		permissions += ATOSE_address_space::WRITE;
	if (current_header->p_flags & PF_X)
		permissions += ATOSE_address_space::EXECUTE;

	/*
		Make sure the address space of the process includes the page we're about to use.
		As we're in the address space of the new process we the add() method returns
		a pointer that is also in our address space so we can simply copy into it.
	*/
	into = process->address_space.add((void *)(current_header->p_vaddr), current_header->p_memsz, permissions);

	/*
		We might fail at this point for seveal reasons; for example, we
		might be out of pages or we might be trying to change the
		permissions on an existing page
	*/
	if (into == NULL)
		{
		process->address_space.destroy();
		return ELF_BAD_OUT_OF_PAGES;
		}

	/*
		Move on to the next segment
	*/	
	current_header++;
	}

/*
	The address space we just created has ATOSE in it so we can
	assume that address space and continue running from there.
*/
mmu->assume(&process->address_space);

/*
	Now load each executable file segment into the new address space
*/
current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	/*
		copy from the ELF file into the address space
	*/
	memcpy((void *)current_header->p_vaddr, file + current_header->p_offset, current_header->p_filesz);

	/*
		Move on to the next segment
	*/	
	current_header++;
	}

/*
	Get out of this process's address space
*/
mmu->assume_identity();

/*
	Extract the process entry point (that is, the location we are to
	branch to for the process to start
*/
process->entry_point = (uint8_t *)header->e_entry;

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
os->io.hex();
os->io << "CALL TO START A NEW PROCESS:\n";

answer = elf_load(&active_process, buffer, length);

os->io << "EFL_load responds:" << answer << "\r\n";

{
for (uint32_t pp = 0; pp < 4096; pp++)
	if (active_process.address_space.get_page_table()[pp] != 0)
		os->io << pp << ":" << active_process.address_space.get_page_table()[pp] << "\r\n";
}

os->io << "Run\r\n";

/*
	Set up the stack-pointer (ARM register R13)
*/
active_process.stack_pointer = (uint8_t *)mmu->highest_address;
active_process.execution_path.registers.r13 = (uint32_t)(active_process.stack_pointer - 512);		// for testing we'll bring it down a bit

/*
	The process will enter whereever the link-register (ARM register R14_current) points once the scheduler schedules the process to be run
*/
active_process.execution_path.registers.r14 = (uint32_t)active_process.entry_point;		// DELETE ME
active_process.execution_path.registers.r14_current = (uint32_t)active_process.entry_point;

/*
	When we do run we need to return to user-mode which is done by setting the CPSR register's initial value
*/
active_process.execution_path.registers.cpsr = 0x80000150;						// flags including processor mode (e.g. USER / IRQ mode)
os->io << "ACTIVE_PROCESS:" << (uint32_t)&active_process << "\r\n";
os->scheduler.push(&active_process);

return length;
}

