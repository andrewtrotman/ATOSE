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
#include "atose_api.h"
#include "ascii_str.h"
#include "elf32_ehdr.h"
#include "elf32_phdr.h"
#include "process_manager.h"


#include "server_disk.h"
#include "server_disk_protocol.h"
#include "ascii_str.h"
#include "file_control_block.h"

#include "client_file.h"

void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);

/*
	ATOSE_PROCESS_MANAGER::INITIALISE()
	-----------------------------------
*/
void ATOSE_process_manager::initialise(ATOSE_mmu *mmu, ATOSE_process_allocator *process_allocator)
{
this->mmu = mmu;
active_head = active_tail = NULL;
current_process = 0;
system_address_space = process_allocator->malloc_address_space();
//system_address_space->create_identity();
system_address_space->create();
system_address_space->the_heap_break = mmu->the_system_break;
}

/*
	ATOSE_PROCESS_MANAGER::ELF_LOAD()
	---------------------------------
	We need to check as much of this as we can because this is an obvious way
	to attack the kernel.

	Return 0 on success all other codes are error codes
	on success this method sets *entry_point to the start of the executable
*/
uint32_t ATOSE_process_manager::elf_load(ATOSE_FILE *infile, void **entry_point)
{
ATOSE_elf32_ehdr header;          // the ELF file header
uint32_t header_ok;               // used to check the ELf magic number is correct
uint32_t header_offset;           // location (in the file) of the first header
uint32_t header_size;             // size of each header
uint32_t header_num;              // number of headers
ATOSE_elf32_phdr current_header;  // the current header
uint32_t which;                   // which header we're currently looking at
uint32_t permissions;             // permissions on pages in the address space (READ / WRITE / EXECUTE / etc.)
uint32_t error;                   // has an error occurred while processing a segment?
uint64_t file_offset;             // the location of the next program header in the file
uint8_t *current_break;           // current of the break as it shifts as a consequence of loading the ELF file.
uint32_t shift;                   // the amount we need to shift the break

/*
	We must be able to read from the ELF file.
*/
if (ATOSE_fread(&header, sizeof(header), 1, infile) == 0)
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
	Get the current break
*/
current_break = ATOSE_api::get_heap_break();

/*
	Verify and load the elf file.
*/
file_offset = header_offset;
error = SUCCESS;
for (which = 0; which < header_num; which++)
	{
	ATOSE_fseek(infile, file_offset, ATOSE_SEEK_SET);
	if (ATOSE_fread(&current_header, sizeof(current_header), 1, infile) == 0)
		error = ELF_BAD_PROGRAM_HEADER_LOCATION;
	file_offset += sizeof(current_header);

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
		{
		if ((uint8_t *)(current_header.p_vaddr + current_header.p_memsz) > current_break)
			{
			shift = ((uint8_t *)current_header.p_vaddr) + current_header.p_memsz - current_break;
ATOSE_api::writeline("call set_heap_break[");
			ATOSE_api::set_heap_break(shift, permissions);
ATOSE_api::writeline("]");
			current_break = (uint8_t *)(current_header.p_vaddr + current_header.p_memsz);
			}
		}

	/*
		We might fail at this point for several reasons; for example, we
		might be out of pages or we might be trying to change the
		permissions on an existing page
	*/
	if (error != SUCCESS)
		{
ATOSE_api::writeline("fail!");
		break;
		}

	/*
		copy from the ELF file into the address space
	*/
	ATOSE_fseek(infile, current_header.p_offset, ATOSE_SEEK_SET);

void debug_print_cf_this(const char *start, uint32_t hex1, uint32_t hex2, const char *end = "");
debug_print_cf_this("[offset->", current_header.p_offset, current_header.p_filesz, "<-bytes]\r\n");

	ATOSE_fread((void *)current_header.p_vaddr, current_header.p_filesz, 1, infile);

//	debug_dump_buffer((unsigned char *)current_header.p_vaddr, current_header.p_vaddr, 0x80);
	}

/*
	Extract the process entry point (that is, the location we are to
	branch to for the process to start
*/

*entry_point = (uint8_t *)header.e_entry;

//ATOSE_atose::get_ATOSE()->debug.hex();
//ATOSE_atose::get_ATOSE()->debug << "e_entry=" << header.e_entry << "\r\n";

return error;
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
uint32_t ATOSE_process_manager::initialise_process(ATOSE_process *process, void *entry_point, uint32_t mode, uint32_t top_of_stack)
{
/*
	Set up the stack-pointer (ARM register R13)
*/
process->execution_path->registers.r13 = top_of_stack & ((uint32_t)~0x03);		// align it correctly (rounding down)

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
	ATOSE_PROCESS_MANAGER::EXEC()
	-----------------------------
*/
uint32_t ATOSE_process_manager::exec(uint32_t parameter)
{
uint32_t pipe, got;
ATOSE_FILE *fcb;
void *entry_point;

pipe = ATOSE_api::pipe_create();
do
	if ((got = ATOSE_api::pipe_connect(pipe, ATOSE_server_disk::PIPE)) == 0)
		ATOSE_api::yield();		// give up my time slice to someone else
while (got != 0);

if ((fcb = ATOSE_fopen((char *)parameter, "r+b")) != 0)
	{
	got = ATOSE_atose::get_ATOSE()->scheduler.elf_load(fcb, &entry_point);
	ATOSE_fclose(fcb);
	if (got == SUCCESS)
		{
		/*
			Drop into user mode and branch to the entry point
		*/
		asm volatile
			(
			"mrs r0, cpsr;"
			"bic r0, #0x1F;"
			"orr r0, r0, #0x10;"
			"msr cpsr, r0;"
			:
			:
			: "r0"
			);

		(*((void (*)(void))entry_point))();
		}
	}

ATOSE_api::exit(0);
return 0;
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
uint8_t filename[ATOSE_file_control_block::MAX_PATH];

/*
	Copy the filename out of the caller's address space into kernel space
	because we are going to want to swap address spaces a few times but
	will need this the whole time.
*/
ASCII_strncpy((char *)filename, (char *)elf_filename, ATOSE_file_control_block::MAX_PATH);
filename[ATOSE_file_control_block::MAX_PATH - 1] = '\0';

/*
	Create a new process
*/
if ((new_process = ATOSE_atose::get_ATOSE()->process_allocator.malloc()) == NULL)
	return ELF_BAD_OUT_OF_PROCESSES;

/*
	Create the address space
*/
if (new_process->address_space->create() == 0)
	return ELF_BAD_ADDRESS_SPACE_FAIL;

/*
	Set up the register set
*/
answer = initialise_process(new_process, (void *)exec, ATOSE_cpu_arm::MODE_SYSTEM, mmu->highest_address);

/*
	Copy the command line onto the top of the stack
	Put a pointer to the filename in R0
	Then realign the stack
*/
mmu->assume(new_process->address_space);
new_process->execution_path->registers.r13 -= ASCII_strlen(filename) + 1;
ASCII_strcpy((char *)new_process->execution_path->registers.r13 , filename);
new_process->execution_path->registers.r0 = new_process->execution_path->registers.r13;
while (new_process->execution_path->registers.r13 & 0x03)
	new_process->execution_path->registers.r13--;

/*
	Add to the process queues (for the scheduler to sort out)
*/
push(new_process);

return answer;
}


/*
	ATOSE_PROCESS_MANAGER::CREATE_THREAD()
	--------------------------------------
*/
uint32_t ATOSE_process_manager::create_thread(uint32_t (*start)(void))
{
uint32_t answer;
ATOSE_process *new_process;
uint8_t *old_stack_break;
ATOSE_address_space *address_space = system_address_space;

if ((new_process = ATOSE_atose::get_ATOSE()->process_allocator.malloc(address_space)) == NULL)
	return ELF_BAD_OUT_OF_PROCESSES;

/*
	Add one to the address-space's reference count
*/
address_space->get_reference();

/*
	Give it a local stack
*/
old_stack_break = address_space->the_stack_break;

address_space->the_stack_break -= mmu->page_size;
address_space->add(address_space->the_stack_break, mmu->page_size, ATOSE_address_space::WRITE | ATOSE_address_space::READ);
answer = initialise_process(new_process, (void *)start, ATOSE_cpu_arm::MODE_SYSTEM, (uint32_t)old_stack_break);

push(new_process);

return answer;
}

/*
	ATOSE_PROCESS_MANAGER::CREATE_SYSTEM_THREAD()
	---------------------------------------------
*/
uint32_t ATOSE_process_manager::create_system_thread(uint32_t (*start)(void), const char *name, uint32_t is_idle_thread)
{
uint32_t answer;
ATOSE_process *new_process;
uint8_t *old_stack_break;

if ((new_process = ATOSE_atose::get_ATOSE()->process_allocator.malloc(system_address_space)) == NULL)
	return ELF_BAD_OUT_OF_PROCESSES;

/*
	Add one to the address-space's reference count
*/
system_address_space->get_reference();

/*
	Give it a local stack
*/
old_stack_break = system_address_space->the_stack_break;
system_address_space->the_stack_break -= mmu->page_size;
system_address_space->add(system_address_space->the_stack_break, mmu->page_size, ATOSE_address_space::WRITE | ATOSE_address_space::READ);

answer = initialise_process(new_process, (void *)start, ATOSE_cpu_arm::MODE_SYSTEM, (uint32_t)old_stack_break);

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
ATOSE_process *next_process;

/*
	What's next to run?
*/
if ((next_process = get_next_process()) == NULL)
	{
	/*
		We don't have any more work to do so we context switch the idle process
	*/
	memcpy(registers, &idle->execution_path->registers, sizeof(*registers));
	mmu->assume(idle->address_space);
	}
else
	{
	/*
		Context switch to a queued process
		Set the registers and the address space for the new process
	*/
	memcpy(registers, &next_process->execution_path->registers, sizeof(*registers));
	mmu->assume(next_process->address_space);
	}

return 0;
}
