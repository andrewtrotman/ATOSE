/*
	API_ATOSE.H
	-----------
*/
#ifndef API_ATOSE_H_
#define API_ATOSE_H_

#include <stdint.h>
#include "registers.h"
#include "io.h"

#define ATOSE_SWI 0x6174

/*
	class ATOSE_API
	---------------
*/
class ATOSE_API
{
public:
	enum { id_object_none = 0, id_object_keyboard = 'k', id_object_mouse = 'm', id_object_serial = 's', id_object_process_manager = 'p'};
	enum { id_function_read_byte = 'r', id_function_write_byte = 'w', id_function_write_block = 'W' };
} ;

/*
	class ATOSE_API_io
	------------------
	the object id is passed in r0
	the method id is passed in r1

	the return code is passed back in r0
*/
class ATOSE_API_io : public ATOSE_API, public ATOSE_IO
{
public:
	uint32_t id_object;

public:
	ATOSE_API_io()	{ id_object = id_object_none; }

	virtual uint32_t read_byte(uint8_t *buffer)
	{
	uint32_t answer;
	uint32_t the_byte;

	asm volatile 
		(
		"mov r0, %[object];"
		"mov r1, %[function];"
		"swi %[ATOSE_swi];"
		"mov %[answer], r0;"
		"mov %[the_byte], r1;"
		: [answer]"=r"(answer), [the_byte]"=r"(the_byte)
		: [object]"r"(id_object), [function]"r"(id_function_read_byte), [ATOSE_swi]"i"(ATOSE_SWI)
		: "r0", "r1"
		);

	*buffer = (uint8_t)the_byte;
	return answer;
	}

	virtual uint32_t write_byte(const uint8_t buffer)
	{
	uint32_t answer;

	asm volatile 
		(
		"mov r0, %[object];"
		"mov r1, %[function];"
		"mov r2, %[buffer];"
		"swi %[ATOSE_swi];"
		"mov %[answer], r0;"
		: [answer]"=r" (answer)
		: [object]"r"(id_object), [function]"r"(id_function_write_byte), [buffer]"r"(buffer), [ATOSE_swi]"i"(ATOSE_SWI)
		: "r0", "r1", "r2"
		);

	return answer;
	}

	virtual uint32_t write_block(const uint8_t *buffer, uint32_t length)
	{
	uint32_t answer;

	asm volatile 
		(
		"mov r0, %[object];"
		"mov r1, %[function];"
		"mov r2, %[buffer];"
		"mov r3, %[length];"
		"swi %[ATOSE_swi];"
		"mov %[answer], r0;"
		: [answer]"=r" (answer)
		: [object]"r"(id_object), [function]"r"(id_function_write_block), [buffer]"r"(buffer), [length]"r"(length), [ATOSE_swi]"i"(ATOSE_SWI)
		: "r0", "r1", "r2", "r3"
		);

	return answer;
	}

} ;

#ifdef QEMU
	/*
		class ATOSE_API_MOUSE
		---------------------
	*/
	class ATOSE_API_mouse : public ATOSE_API_io
	{
	public:
		ATOSE_API_mouse() : ATOSE_API_io() { id_object = id_object_mouse; init(); }
	} ;

	/*
		class ATOSE_API_KEYBOARD
		------------------------
	*/
	class ATOSE_API_keyboard : public ATOSE_API_io
	{
	public:
		ATOSE_API_keyboard() : ATOSE_API_io() { id_object = id_object_keyboard; init(); }
	} ;

#endif

/*
	class ATOSE_API_SERIAL
	----------------------
*/
class ATOSE_API_serial : public ATOSE_API_io
{
public:
	ATOSE_API_serial() : ATOSE_API_io() { id_object = id_object_serial; init(); }
} ;

/*
	class ATOSE_API_PROCESS_MANAGER
	-------------------------------
*/
class ATOSE_API_process_manager : public ATOSE_API_io
{
public:
	ATOSE_API_process_manager() : ATOSE_API_io() { id_object = id_object_process_manager; init(); }
} ;


/*
	class ATOSE_API_ATOSE
	---------------------
*/
class ATOSE_API_ATOSE
{
public:
#ifdef QEMU
	ATOSE_API_mouse mouse;
	ATOSE_API_keyboard keyboard;
#endif
	ATOSE_API_serial io;
	ATOSE_API_process_manager process_manager;
} ;


#endif /* API_ATOSE_H_ */
