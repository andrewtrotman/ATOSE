/*
	SERVER_DISK.C
	-------------
*/
#include "atose.h"
#include "server_disk.h"
#include "atose_api.h"
#include "server_disk_protocol.h"
#include "file_control_block.h"

/*
	ATOSE_SERVER_DISK::SERVE()
	--------------------------
*/
uint32_t ATOSE_server_disk::serve(void)
{
uint32_t pipe, message, client_id;
ATOSE_file_control_block *fcb, fcb_space;
ATOSE_server_disk_protocol command;

pipe = ATOSE_api::pipe_create();
ATOSE_api::pipe_bind(pipe, PIPE);

while (1)
	{
	if ((message = ATOSE_api::pipe_receive(pipe, &command, sizeof(command), &client_id)) != 0)
		{
		switch (command.command)
			{
			case ATOSE_server_disk_protocol::COMMAND_OPEN:
				command.filename[sizeof(command.filename) - 1] = '\0';		// make sure its terminated correctly
#ifdef NEVER
				if ((fcb = ATOSE_atose::get_ATOSE()->file_system.open(&fcb_space, command.filename)) == NULL)
					{
//					ATOSE_api::writeline("FILE NOT FOUND");
					}
				else
					{
//					ATOSE_api::writeline("FILE NOW OPEN");
					}
#endif
				ATOSE_api::pipe_reply(message, NULL, 0, (uint32_t)fcb);
				break;

			default:
				ATOSE_api::pipe_reply(message, NULL, 0, 0);
				break;
			}
		}
	}

ATOSE_api::exit(0);
return 0;
}
