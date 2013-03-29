/*
	SERVER_DISK.H
	-------------
*/
#ifndef SERVER_DISK_H_
#define SERVER_DISK_H_

#include <stdint.h>
#include "stdpipe.h"

/*
	class ATOSE_SERVER_DISK
	-----------------------
*/
class ATOSE_server_disk
{
public:
	static const uint32_t PIPE = PIPE_FILE;

public:
	uint32_t serve(void);
} ;

#endif
