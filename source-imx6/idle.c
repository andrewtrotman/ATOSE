/*
	IDLE.C
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This is the ATOSE IDLE process.
*/
#include "idle.h"
#include "atose_api.h"

/*
	IDLE()
	------
*/
uint32_t idle(void)
{
/*
	The only way to get here is to run out of work.  If this happens we can stop
	until we get work to do. That can only happen for a small number of reasons,
	most of which involve an interrupt but some of which involve and "event" such
	as another CPU giving us something to do.  So we use WFE (Wait For Event) which
	does this:

	"If the event register is not set, the processor suspends execution (Clock is stopped) until one of the following events take place:
		*An IRQ interrupt, unless masked by the CPSR I Bit
		*An FIQ interrupt, unless masked by the CPSR F Bit
		*A Debug Entry request made to the processor and Debug is enabled
		*An event is signaled by another processor using Send Event.
		*Another MP11 CPU return from exception."
*/
while (1)
	{
	asm volatile
		(
		"wfe;"					// wait for event
		:
		:
		:
		);
	}

return 0;
}
