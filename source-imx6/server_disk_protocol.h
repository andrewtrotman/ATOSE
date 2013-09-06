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
	enum {COMMAND_SEEK, COMMAND_TELL, COMMAND_READ, COMMAND_WRITE, COMMAND_OPEN, COMMAND_CLOSE};

public:
	uint32_t command;
	uint32_t fcb;
	int64_t location;			// file offsets can be negative
	uint64_t bytes;
	uint64_t times;
	uint8_t from;
	uint8_t filename[255];
	uint8_t mode[4];
} __attribute__ ((packed));

#endif
