/*
	SERVER_DISK.C
	-------------
*/
#include "atose.h"
#include "server_disk.h"
#include "atose_api.h"
#include "server_disk_protocol.h"
#include "file_control_block.h"
#include "malloc.h"
#include "client_file.h"

/*
	class ATOSE_SERVER_DISK_FILE
	----------------------------
*/
class ATOSE_server_disk_file
{
public:
	ATOSE_server_disk_file *next;
	uint32_t process_id;
	ATOSE_file_control_block fcb_space;
	uint8_t buffer;
} __attribute__ ((packed));

/*
	ATOSE_SERVER_DISK::SERVE()
	--------------------------
*/
uint32_t ATOSE_server_disk::serve(void)
{
uint64_t bytes;
uint32_t pipe, message, client_id;
ATOSE_file_control_block *fcb;
ATOSE_server_disk_protocol command;
ATOSE_server_disk_file *file, *open_files = NULL;
ATOSE_atose *os = ATOSE_atose::get_ATOSE();

pipe = ATOSE_api::pipe_create();
ATOSE_api::pipe_bind(pipe, PIPE);

while (1)
	{
	message = ATOSE_api::pipe_receive(pipe, &command, sizeof(command), &client_id);

	if (message != 0)
		{
		/*
			Extract the FCB from the message.  It should really be checked too.
		*/
		fcb = (ATOSE_file_control_block *)command.fcb;

		switch (command.command)
			{
			case ATOSE_server_disk_protocol::COMMAND_OPEN:
				ATOSE_api::writeline("[COMMAND_OPEN]\r\n");
				command.filename[sizeof(command.filename) - 1] = '\0';		// make sure its terminated correctly

				file = (ATOSE_server_disk_file *)KR_malloc(sizeof(*file));
				if ((fcb = os->file_system.open(&file->fcb_space, command.filename)) == NULL)
					KR_free(file);
				else
					{
					fcb->buffer = (uint8_t *)KR_malloc(fcb->block_size_in_bytes);
					file->process_id = client_id;
					file->next = open_files;
					open_files = file;
					}
				ATOSE_api::pipe_reply(message, NULL, 0, (uint32_t)fcb);
				break;

			case ATOSE_server_disk_protocol::COMMAND_CLOSE:
				ATOSE_api::writeline("[COMMAND_CLOSE]\r\n");
				fcb->close();
				/*
					This should remove the fcb from the list and KR_free() fcb->buffer and the fcb itself.
				*/
				ATOSE_api::pipe_reply(message, NULL, 0, 0);
				break;

			case ATOSE_server_disk_protocol::COMMAND_SEEK:
				ATOSE_api::writeline("[COMMAND_SEEK]\r\n");
				if (command.from == ATOSE_SEEK_CUR)
					fcb->seek(fcb->tell() + command.location);
				else if (command.from == ATOSE_SEEK_SET)
					fcb->seek(command.location);
				ATOSE_api::pipe_reply(message, NULL, 0, 0);
				break;

			case ATOSE_server_disk_protocol::COMMAND_TELL:
				ATOSE_api::writeline("[COMMAND_TELL]\r\n");
				command.location = fcb->tell();
				ATOSE_api::pipe_reply(message, &command, sizeof(command), 0);
				break;

			case ATOSE_server_disk_protocol::COMMAND_READ:
				ATOSE_api::writeline("[COMMAND_READ]\r\n");

				{
				uint8_t buffer[1024];
				uint64_t offset;

				bytes = command.bytes * command.times;
				offset = 0;
				while (bytes > sizeof(buffer))
					{
					fcb->read(buffer, sizeof(buffer));
					ATOSE_api::pipe_memcpy(message, offset, buffer, sizeof(buffer));
					offset += sizeof(buffer);
					bytes -= sizeof(buffer);
					}
				if (bytes > 0)
					{
					fcb->read(buffer, bytes);
					ATOSE_api::pipe_memcpy(message, offset, buffer, bytes);
					}
				ATOSE_api::pipe_reply(message, NULL, 0, bytes);
				}
				break;

			default:
				ATOSE_api::writeline("[UNKNOWN MESSAGE!]\r\n");
				ATOSE_api::pipe_reply(message, NULL, 0, 0);
				break;
			}
		}
	}

ATOSE_api::exit(0);
return 0;
}
