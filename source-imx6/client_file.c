/*
	CLIENT_FILE.C
	-------------
*/
#include "ascii_str.h"
#include "atose_api.h"
#include "pipe.h"
#include "client_file.h"
#include "server_disk.h"
#include "server_disk_protocol.h"

static uint32_t initialised = false;
uint32_t ATOSE_file_pipe;

/*
	ATOSE_FINITIALISE()
	-------------------
*/
void ATOSE_finitialise(void)
{
uint32_t got;

ATOSE_file_pipe = ATOSE_api::pipe_create();

do
	if ((got = ATOSE_api::pipe_connect(ATOSE_file_pipe, ATOSE_server_disk::PIPE)) == 0)
		ATOSE_api::yield();		// give up my time slice to someone else
while (got != 0);
}

/*
	ATOSE_FOPEN()
	-------------
*/
ATOSE_FILE *ATOSE_fopen(const char *path, const char *mode)
{
ATOSE_server_disk_protocol command;

if (!initialised)
	ATOSE_finitialise();

command.command = ATOSE_server_disk_protocol::COMMAND_OPEN;

ASCII_strncpy((char *)command.filename, path, sizeof(command.filename));
ASCII_strncpy((char *)command.mode, mode, sizeof(command.mode));

return (ATOSE_FILE *)ATOSE_api::pipe_send(ATOSE_file_pipe, &command, sizeof(command), &command, sizeof(command));
}

/*
	ATOSE_FCLOSE()
	--------------
*/
int32_t ATOSE_fclose(ATOSE_FILE *stream)
{
ATOSE_server_disk_protocol command;

command.command = ATOSE_server_disk_protocol::COMMAND_CLOSE;
command.fcb = (uint32_t)stream;

return ATOSE_api::pipe_send(ATOSE_file_pipe, &command, sizeof(command), &command, sizeof(command));
}

/*
	ATOSE_FSEEK()
	-------------
*/
int32_t ATOSE_fseek(ATOSE_FILE *stream, int64_t offset, uint8_t from)
{
ATOSE_server_disk_protocol command;

command.command = ATOSE_server_disk_protocol::COMMAND_SEEK;
command.fcb = (uint32_t)stream;
command.location = offset;
command.from = from;

return ATOSE_api::pipe_send(ATOSE_file_pipe, &command, sizeof(command), &command, sizeof(command));
}

/*
	ATOSE_FTELL()
	-------------
*/
uint64_t ATOSE_ftell(ATOSE_FILE *stream)
{
ATOSE_server_disk_protocol command;

command.command = ATOSE_server_disk_protocol::COMMAND_TELL;
command.fcb = (uint32_t)stream;

ATOSE_api::pipe_send(ATOSE_file_pipe, &command, sizeof(command), &command, sizeof(command));

return command.location;
}

/*
	ATOSE_FREAD()
	-------------
*/
uint32_t ATOSE_fread(void *destination, uint64_t size, uint32_t count, ATOSE_FILE *stream)
{
ATOSE_server_disk_protocol command;

command.command = ATOSE_server_disk_protocol::COMMAND_READ;
command.fcb = (uint32_t)stream;
command.bytes = size;
command.times = count;

return ATOSE_api::pipe_send(ATOSE_file_pipe, destination, size * count, &command, sizeof(command));
}
