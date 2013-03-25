/*
	PROCESS_MANAGER.C
	-----------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>
#include "atose.h"
#include "cpu_arm.h"
#include "mmu_page.h"
#include "ascii_str.h"
#include "elf32_ehdr.h"
#include "elf32_phdr.h"
#include "process_manager.h"

void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);

/*
	ATOSE_PROCESS_MANAGER::ATOSE_PROCESS_MANAGER()
	----------------------------------------------
*/
ATOSE_process_manager::ATOSE_process_manager(ATOSE_mmu *mmu, ATOSE_process_allocator *process_allocator)
{
this->mmu = mmu;
active_head = active_tail = NULL;
current_process = 0;
system_address_space = process_allocator->malloc_address_space();
system_address_space->create_identity();
}

/*
	ATOSE_PROCESS_MANAGER::ELF_LOAD()
	---------------------------------
	We need to check as much of this as we can because this is an obvious way
	to attack the kernel.

	Return 0 on success all other codes are error codes
*/
uint32_t ATOSE_process_manager::elf_load(ATOSE_process *process, ATOSE_file_control_block *infile)
{
ATOSE_elf32_ehdr header;          // the ELF file header
uint32_t header_ok;               // used to check the ELf magic number is correct
uint32_t header_offset;           // location (in the file) of the first header
uint32_t header_size;             // size of each header
uint32_t header_num;              // number of headers
ATOSE_elf32_phdr current_header;  // the current header
uint32_t which;                   // which header we're currently looking at
uint32_t permissions;             // permissions on pages in the address space (READ / WRITE / EXECUTE / etc.)
uint8_t *into;                    // pointer to a segment returned by the address space
uint32_t error;                   // has an error occurred while processing a segment?
uint64_t file_offset;             // the location of the next program header in the file

/*
	We must be able to read from the ELF file.
*/
if (infile->read(&header, sizeof(header)) == 0)
	return ELF_BAD_FROM_START;

/*
	First verify that we have and ELF file.
*/
header_ok = 0;
header_ok += (header.e_ident[ATOSE_elf32_ehdr::EI_MAG0] == 0x7F);
header_ok += (header.e_ident[ATOSE_elf32_ehdr::EI_MAG1] == 'E');
header_ok += (header.e_ident[ATOSE_elf32_ehdr::EI_MAG2] == 'L');
header_ok += (header.e_ident[ATOSE_elf32_ehdr::EI_MAG3] == 'F');

if (header_ok != 4)
	return ELF_BAD_ID;

/*
	Check we have a 32-bit file
*/
if (header.e_ident[ATOSE_elf32_ehdr::EI_CLASS] != ATOSE_elf32_ehdr::ELFCLASS32)
	return ELF_BAD_BITNESS;

/*
	Check we have a little endian file
*/
if (header.e_ident[ATOSE_elf32_ehdr::EI_DATA] != ATOSE_elf32_ehdr::ELFDATA2LSB)
	return ELF_BAD_ENDIAN;

/*
	Check we have a file version we understand
*/
if (header.e_ident[ATOSE_elf32_ehdr::EI_VERSION] != ATOSE_elf32_ehdr::EV_CURRENT)
	return ELF_BAD_VERSION;

/*
	Make sure the header is the right length
*/
if (header.e_ehsize != sizeof(ATOSE_elf32_ehdr))
	return ELF_BAD_HEADER;

/*
	Make sure the header is a version we understand
*/
if (header.e_version != ATOSE_elf32_ehdr::EV_CURRENT)
	return ELF_BAD_HEADER_VERSION;

/*
	Now check we're an executable file
*/
if (header.e_type != ATOSE_elf32_ehdr::ET_EXEC)
	return ELF_BAD_TYPE;

/*
	Check we're an ARM executable file
*/
if (header.e_machine != ATOSE_elf32_ehdr::EM_ARM)
	return ELF_BAD_ARCHITECTURE;

/*
	We passed the tests so at this point we think we have an ELF file
	thats ARM 32-bit little endian.  Of course, it might just be chance
	(or a filed designed to fool) that we this far so we're not out of
	hot water yet.

*/
if (header.e_phnum == 0)
	return ELF_BAD_PROGRAM_HEADER;

/*
	Now we move on to the ELF program header
*/
header_offset = header.e_phoff;
header_size = header.e_phentsize;
header_num = header.e_phnum;

/*
	Make sure the program header is the righ size
*/
if (header_size != sizeof(ATOSE_elf32_phdr))
	return ELF_BAD_PROGRAM_HEADER_SIZE;

#ifdef NEVER
	/*
		This code is removed because we can't check this without knowing how big the file is.
	*/
/*
	Make sure the program header is located within the file itself
	this is an obvious attach we can catch
*/
if ((header_offset + header_size * header_num) > length)
	return ELF_BAD_PROGRAM_HEADER_LOCATION;
#endif

/*
	We passed all the initial tests so we create the address space
*/
if (process->address_space->create() == 0)
	return ELF_BAD_ADDRESS_SPACE_FAIL;

/*
	The address space we just created has ATOSE in it so we can
	assume that address space and continue running from there.
*/
mmu->assume(process->address_space);

/*
	Verify and load the elf file.
*/
file_offset = header_offset;
for (which = 0; which < header_num; which++)
	{
	error = SUCCESS;

	infile->seek(file_offset);
	if (infile->read(&current_header, sizeof(current_header)) == 0)
		error = ELF_BAD_PROGRAM_HEADER_LOCATION;
	file_offset = infile->tell();
	/*
		At present we only allow loadable segments (program and data) and
		C++ stack unwind semantics segments.
	*/
	if (current_header.p_type != ATOSE_elf32_phdr::PT_ARM_UNWIND && current_header.p_type != ATOSE_elf32_phdr::PT_LOAD)
		error = ELF_BAD_PROGRAM_SEGMENT_TYPE;

	/*
		Make sure that in all cases the size in the ELF file is no larger than the amount
		of memory needed to store it.
	*/
	if (current_header.p_filesz > current_header.p_memsz)
		error = ELF_BAD_PROGRAM_SEGMENT_SIZE;

#ifdef NEVER
	/*
		Make sure we don't read past end of file
	*/
	if (current_header.p_offset + current_header.p_filesz > length)
		return ELF_BAD_PROGRAM_SEGMENT_LOCATION;
#endif

	/*
		Get the page permissions (this code is somewhat redundant because
		we've set up the permission flags in address_space->add() to
		match those of the ELF so that we can use current_header->p_flags
		without the translation below.  In the mean time we leave this
		code in here.
	*/
	permissions = ATOSE_address_space::NONE;
	if (current_header.p_flags & ATOSE_elf32_phdr::PF_R)
		permissions += ATOSE_address_space::READ;
	if (current_header.p_flags & ATOSE_elf32_phdr::PF_W)
		permissions += ATOSE_address_space::WRITE;
	if (current_header.p_flags & ATOSE_elf32_phdr::PF_X)
		permissions += ATOSE_address_space::EXECUTE;

	/*
		Make sure the address space of the process includes the page we're about to use.
		As we're in the address space of the new process we the add() method returns
		a pointer that is also in our address space so we can simply copy into it.
	*/
	if (error == SUCCESS)
		if ((into = process->address_space->add((void *)(current_header.p_vaddr), current_header.p_memsz, permissions)) == NULL)
			error = ELF_BAD_OUT_OF_PAGES;
	/*
		When we change the address space we need to make sure the CPU can see it... we have to (at a minimum) flush the TLB
	*/
	mmu->assume(process->address_space);

	/*
		We might fail at this point for seveal reasons; for example, we
		might be out of pages or we might be trying to change the
		permissions on an existing page
	*/
	if (error != SUCCESS)
		{
		mmu->assume_identity();
		process->address_space->destroy();
		return error;
		}

	/*
		copy from the ELF file into the address space
	*/
	infile->seek(current_header.p_offset);
	infile->read((void *)current_header.p_vaddr, current_header.p_filesz);

debug_print_string("\r\n");
debug_dump_buffer((unsigned char *)current_header.p_vaddr, current_header.p_vaddr, 0x80);
	}

/*
	Get out of this process's address space
*/
mmu->assume_identity();

/*
	Extract the process entry point (that is, the location we are to
	branch to for the process to start
*/

process->entry_point = (uint8_t *)header.e_entry;

ATOSE_atose::get_ATOSE()->debug.hex();
ATOSE_atose::get_ATOSE()->debug << "e_entry=" << header.e_entry << "\r\n";

return SUCCESS;
}

/*
	ATOSE_SCHEDULE::PUSH()
	----------------------
	The process list is a queue
*/
void ATOSE_process_manager::push(ATOSE_process *process)
{
/*
	Make sure we don't push NULL (so that we can go push(pull())
	when pull() returns NULL,
*/
if (process == NULL)
	return;

if (active_head != NULL)
	active_head->next = process;	// the queue wasn't empty

process->next = NULL;

active_head = process;

if (active_tail == NULL)
	active_tail = process;
}

/*
	ATOSE_SCHEDULE::PULL()
	----------------------
	The process list is a queue
*/
ATOSE_process *ATOSE_process_manager::pull(void)
{
ATOSE_process *answer;

if (active_tail == NULL)
	return 0;						// the queue is already empty

answer = active_tail;

if ((active_tail = active_tail->next) == NULL)
	active_head = NULL;				// the queue is now empty

return answer;
}

/*
	ATOSE_PROCESS_MANAGER::GET_NEXT_PROCESS()
	-----------------------------------------
*/
ATOSE_process *ATOSE_process_manager::get_next_process(void)
{
/*
	To start with we'll just use a simple process queue

	put the current process at the bottom of the queue and replace it with
	the process at the top of the queue
*/
if (current_process != idle)
	push(current_process);
return current_process = pull();
}

/*
	ATOSE_PROCESS_MANAGER::INITIALISE_PROCESS()
	-------------------------------------------
*/
uint32_t ATOSE_process_manager::initialise_process(ATOSE_process *process, size_t entry_point, uint32_t mode, uint32_t top_of_stack)
{
/*
	Set up the stack-pointer (ARM register R13)
*/
process->execution_path->registers.r13 = top_of_stack & ((uint32_t)~0x03);		// align it correctly

/*
	The process will enter where-ever the link-register (ARM register
	R14_current) points once the scheduler schedules the process to be
	run
*/
process->execution_path->registers.r14_current = (uint32_t)entry_point;

/*
	When we do run we need to return to user-mode which is done by setting the CPSR register's low bits
*/
process->execution_path->registers.cpsr = (0x80000150 & ~ATOSE_cpu_arm::MODE_BITS) | mode;

return SUCCESS;
}

/*
	ATOSE_PROCESS_MANAGER::CREATE_PROCESS()
	---------------------------------------
	returns SUCCESS on success, all other values are error codes
*/
uint32_t ATOSE_process_manager::create_process(const uint8_t *elf_filename)
{
uint32_t answer;
ATOSE_process *new_process;

ATOSE_atose::get_ATOSE()->debug << "==\r\n";

/*
	Create a new process
*/
if ((new_process = ATOSE_atose::get_ATOSE()->process_allocator.malloc()) == NULL)
	return ELF_BAD_OUT_OF_PROCESSES;

/*
	Load the ELF file into it
*/
ATOSE_file_control_block *fcb, fcb_space;
if ((fcb = ATOSE_atose::get_ATOSE()->file_system.open(&fcb_space, elf_filename)) == NULL)
	return ELF_BAD_FILE_NOT_FOUND;

uint8_t fcb_buffer[fcb->block_size_in_bytes];
fcb->buffer = fcb_buffer;

answer = elf_load(new_process, fcb);
fcb->close();

if (answer != SUCCESS)
	{
	ATOSE_atose::get_ATOSE()->process_allocator.free(new_process);
	return answer;
	}

/*
	Set up the register set
*/
answer = initialise_process(new_process, (size_t)new_process->entry_point, ATOSE_cpu_arm::MODE_USER, mmu->highest_address);

/*
	Add to the process queues (for the scheduler to sort out)
*/
push(new_process);

ATOSE_atose::get_ATOSE()->debug << "ELF created new process:" << (uint32_t)new_process << "\r\n";
ATOSE_atose::get_ATOSE()->debug << "ELF entry point" << (uint32_t)new_process->entry_point << "(R14=" << new_process->execution_path->registers.r14_current << ")\r\n";


mmu->assume(new_process->address_space);
debug_dump_buffer((unsigned char *)0x10010000, 0x10010000, 0x80);
mmu->assume_identity();

ATOSE_atose::get_ATOSE()->debug << "==\r\n";

return answer;
}

/*
	ATOSE_PROCESS_MANAGER::CREATE_SYSTEM_THREAD()
	---------------------------------------------
*/
uint32_t ATOSE_process_manager::create_system_thread(uint32_t (*start)(void), uint32_t is_idle_thread)
{
uint32_t answer;
ATOSE_process *new_process;
ATOSE_mmu_page *page;

if ((new_process = ATOSE_atose::get_ATOSE()->process_allocator.malloc(system_address_space)) == NULL)
	return ELF_BAD_OUT_OF_PROCESSES;

/*
	Add one to the address-space's reference count
*/
system_address_space->get_reference();

/*
	Give it a local stack
*/
page = system_address_space->add_to_identity();

answer = initialise_process(new_process, (size_t)(start), ATOSE_cpu_arm::MODE_SYSTEM, (uint32_t)(page->physical_address + page->page_size));

/*
	Add to the process queues (for the scheduler to sort out)
*/
if (is_idle_thread)
	set_idle(new_process);
else
	push(new_process);

return answer;
}

/*
	ATOSE_PROCESS_MANAGER::TERMINATE_CURRENT_PROCESS()
	--------------------------------------------------
*/
uint32_t ATOSE_process_manager::terminate_current_process(void)
{
if (current_process != idle)
	{
	ATOSE_atose::get_ATOSE()->process_allocator.free(current_process);
	current_process = NULL;
	}

return 0;
}

/*
	ATOSE_PROCESS_MANAGER::CONTEXT_SWITCH()
	---------------------------------------
*/
uint32_t ATOSE_process_manager::context_switch(ATOSE_registers *registers)
{
ATOSE_process *current_process, *next_process;

/*
	What's running and what's next to run?
*/
current_process = get_current_process();
next_process = get_next_process();

/*
ATOSE_atose::get_ATOSE()->debug.hex();
ATOSE_atose::get_ATOSE()->debug << "[";
ATOSE_atose::get_ATOSE()->debug <<	(uint32_t)current_process << "@";
if (current_process == 0)
	ATOSE_atose::get_ATOSE()->debug << "?->";
else
	ATOSE_atose::get_ATOSE()->debug << current_process->execution_path->registers.r14_current << "->";
ATOSE_atose::get_ATOSE()->debug <<	(uint32_t)next_process << "@" ;
if (next_process == 0)
	ATOSE_atose::get_ATOSE()->debug << "? ";
else
	ATOSE_atose::get_ATOSE()->debug << next_process->execution_path->registers.r14_current << " ";
*/

if (current_process == next_process && next_process != NULL)
	{
	/*
		If the current process is the next process then there is no work to do
	*/
//	ATOSE_atose::get_ATOSE()->debug << "SELF";
	}
else if (next_process == NULL)
	{
	/*
		We don't have any more work to do so we invoke the idle process
	*/
//	ATOSE_atose::get_ATOSE()->debug << "IDLE";
	memcpy(registers, &idle->execution_path->registers, sizeof(*registers));
	ATOSE_atose::get_ATOSE()->heap.assume(idle->address_space);
	}
else
	{
	/*
		If we're running a process then copy the registers into its register space
		this way if we cause a context switch then we've not lost anything
	*/
	if (current_process != NULL)
		{
//		debug_print_string("S");
		memcpy(&current_process->execution_path->registers, registers, sizeof(*registers));
		}

	/*
		Context switch
	*/
	if (next_process != NULL)
		{
//		debug_print_string("R");

		/*
			Set the registers so that we fall back to the next context
		*/
		memcpy(registers, &next_process->execution_path->registers, sizeof(*registers));

		/*
			Set the address space to fall back to the next context, but only if we aren't a thread of the same address space
		*/
		if (current_process == NULL || next_process->address_space != current_process->address_space)
			{
//			debug_print_string("A");

			mmu->assume(next_process->address_space);

/*
			if (registers->r14_current == 0x10010000)
				{
				debug_print_string("\r\n");
				debug_dump_buffer((unsigned char *)registers->r14_current, registers->r14_current, 0x80);
				}
*/
			}
		}
	}
//debug_print_string("]");

return 0;
}
