/*
	SERVER_DISK_PROTOCOL.H
	----------------------
*/
#ifndef SERVER_DISK_PROTOCOL_H_
#define SERVER_DISK_PROTOCOL_H_

#include <stdint.h>

/*
	class ATOSE_SERVER_DISK_PROTOCOL
	--------------------------------
*/
class ATOSE_server_disk_protocol
{
public:
	enum {COMMAND_OPEN, COMMAND_CLOSE, COMMAND_SEEK, COMMAND_TELL, COMMAND_READ, COMMAND_WRITE};

public:
	uint32_t command;
	uint8_t filename[255];
} ;

#endif
