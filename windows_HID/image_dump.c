/*
   IMAGE_DUMP
   ----------
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
	Standard types
*/
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned long uint32_t;
typedef long int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

#define IMX_IMAGE_VERSION 				0x40
#define IMX_IMAGE_FILE_HEADER_LENGTH 0x2000

#define IMX_IMAGE_TAG_FILE_HEADER	 	0xD1
#define IMX_IMAGE_TAG_DCD 				0xD2

#define IMX_IMAGE_DCD_COMMAND_WRITE_DATA	0xCC
#define IMX_IMAGE_DCD_COMMAND_CHECK_DATA	0xCF
#define IMX_IMAGE_DCD_COMMAND_NOP			0xC0
#define IMX_IMAGE_DCD_COMMAND_UNLOCK		0xB2

/*
	IMX_IMAGE_HEADER
   ----------------
*/
#pragma pack(1)
typedef struct
{
uint8_t tag;					// see IMX_IMAGE_TAG_xxx 
uint16_t length;				// BigEndian format
uint8_t version;				// for the i.MX6 this should be either 0x40 or 0x41
} imx_image_header;
#pragma pack()

/*
   IMX_IMAGE_IVT
   -------------
	See page 441-442 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
typedef struct
{
imx_image_header header;		// should be: tag=0xD1, length=0x0020, version=0x40
uint32_t entry;				// Absolute address of the first instruction to execute from the image
uint32_t reserved1;
uint32_t dcd;					// Absolute address of the image DCD. The DCD is optional so this field may be set to NULL if no DCD is required
uint32_t boot_data;			// Absolute address of the Boot Data
uint32_t self;					// Absolute address of the IVT
uint32_t csf;					// Absolute address of Command Sequence File (CSF) used by the HAB library
uint32_t reserved2;
} imx_image_ivt;
#pragma pack()

/*
	IMX_IMAGE_BOOT_DATA
	-------------------
	See page 442 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
typedef struct
{
uint32_t start;		// Absolute address of the image
uint32_t length;		// Size of the program image
uint32_t plugin;		// Plugin flag
} imx_image_boot_data;
#pragma pack()

/*
   IMX_IMAGE_DCD_HEADER
   --------------------
	See page 443-444 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
typedef struct
{
uint8_t tag;			// the DCD command  (0xCC for a write)
uint16_t length;		// BigEndian format
uint8_t parameter;		// This is command specific, but imximage (in  u-boot) appears to only use 0x02 (i.e. 16-bits) for the write command
} imx_image_dcd_header;
#pragma pack()

/*
   IMX_IMAGE_DCD_WRITE_TUPLE
   -------------------------
	See page 443-444 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
typedef struct
{
uint32_t address;
uint32_t value;
} imx_image_dcd_write_tuple;
#pragma pack()


/*
	IMX_IMAGE_DCD_WRITE
	-------------------
*/
#pragma pack(1)
typedef struct 
{
imx_image_dcd_header header;
imx_image_dcd_write_tuple action[];
} imx_image_dcd_write;
#pragma pack()

/*
   IMX_IMAGE_DCD
   -------------
*/
#define MAX_HW_CFG_SIZE_V2 121
#pragma pack(1)
typedef struct
{
imx_image_header header;				// this is the header of the entire DCD
imx_image_dcd_write command;			// the first command in the DCD (should probably be a union based on the header's tag)
} imx_image_dcd;
#pragma pack()

/*
	ARM_TO_INTEL()
	--------------
*/
uint32_t arm_to_intel(uint32_t value)
{
union i_to_b
{
uint32_t number;
struct { unsigned char one, two, three, four; } byte;
} in, out;

in.number = value;
out.byte.one = in.byte.four;
out.byte.two = in.byte.three;
out.byte.three = in.byte.two;
out.byte.four = in.byte.one;

return out.number;
}

/*
	ARM_TO_INTEL()
	--------------
*/
uint16_t arm_to_intel(uint16_t value)
{
union i_to_b
{
uint32_t number;
struct { unsigned char one, two; } byte;
} in, out;

in.number = value;
out.byte.one = in.byte.two;
out.byte.two = in.byte.one;

return out.number;
}

/*
	DUMP_BUFFER()
	-------------
*/
void dump_buffer(unsigned char *buffer, uint64_t address, uint64_t bytes)
{
uint64_t remaining, width, column;

remaining = bytes;
while (remaining > 0)
	{
	printf("%04X: ", (uint32_t)address);

	width = remaining > 0x10 ? 0x10 : remaining;

	for (column = 0; column < width; column++)
		printf("%02X ", buffer[column]);

	for (; column < 0x10; column++)
		printf("   ");

	printf(" ");
	for (column = 0; column < width; column++)
		printf("%c", isprint(buffer[column]) ? buffer[column] : '.');

	puts("");
	buffer += width;
	address += width;
	remaining -= width;
	}
}

/*
	IMX_IMAGE_PROCESS()
	-------------------
*/
int imx_image_process(char *raw_file, uint32_t raw_file_length)
{
imx_image_ivt *file;
imx_image_dcd *dcd;
char *command_end, *command_start, *dcd_end, *raw_file_end = raw_file + raw_file_length;
char *application_start, *application_end;
imx_image_boot_data *boot_data;

/*
	First check that the file header is correct
*/
file = (imx_image_ivt *)raw_file;
if (file->header.tag != IMX_IMAGE_TAG_FILE_HEADER || file->header.length != IMX_IMAGE_FILE_HEADER_LENGTH || file->header.version != IMX_IMAGE_VERSION)
	return 1;	// the file's magic number does not match so we're probably not an imximage file

puts("HEADER");
puts("------");
printf("Entry Point            : 0x%08X\n", file->entry);
printf("Self                   : 0x%08X\n", file->self);
printf("Boot Data Pointer      : 0x%08X\n", file->boot_data);
printf("DCD Pointer            : 0x%08X\n", file->dcd);
printf("CSF Pointer            : 0x%08X\n", file->csf);
puts("");

/*
	Dump out the Boot Data
*/
if (file->boot_data != 0)
	{
	boot_data = (imx_image_boot_data *)(raw_file + file->boot_data - file->self);
	puts("BOOT DATA");
	puts("---------");
	printf("Start                  : 0x%08X\n", boot_data->start);
	printf("Length                 : 0x%08X\n", boot_data->length);
	printf("Plugin                 : 0x%08X\n", boot_data->plugin);
	puts("");
	}

/*
	We then check the dcd header
*/
if (file->dcd != 0)
	{
	dcd = (imx_image_dcd *)(raw_file + file->dcd - file->self);
	if (dcd->header.tag != IMX_IMAGE_TAG_DCD || dcd->header.version != IMX_IMAGE_VERSION)
		return 2;	// the DCD's magic number doesn't is wrong so we're probably not an imximage file

	puts("DCD");
	puts("---");

	dcd_end = ((char *)dcd) + arm_to_intel(dcd->header.length);
	command_start = command_end = (char *)&(dcd->command);
	while(command_start < dcd_end)
		{
		switch (((imx_image_dcd_write *)command_start)->header.tag)
			{
			case IMX_IMAGE_DCD_COMMAND_WRITE_DATA:
				{
				imx_image_dcd_write_tuple *current;

				printf("Command:WRITE_DATA ");
				printf("SubCommand:0x%02X (%X bytes)\n", dcd->command.header.parameter, arm_to_intel(dcd->command.header.length));
				command_start += sizeof(imx_image_dcd_header);
				command_end += arm_to_intel(dcd->command.header.length);
				for (current = (imx_image_dcd_write_tuple *)command_start; (char *)current < command_end; current++)
					printf("   *(0x%08X) = 0x%08X\n", arm_to_intel(current->address), arm_to_intel(current->value));
				break;
				}
			case IMX_IMAGE_DCD_COMMAND_CHECK_DATA: puts("Check Data"); break;
			case IMX_IMAGE_DCD_COMMAND_NOP: puts("NOP"); break;
			case IMX_IMAGE_DCD_COMMAND_UNLOCK: puts("Unlock"); break;
			}
		command_start = command_end;
		}
	puts("");
	}

/*
	Now the application data (i.e. the binary we're going to run)
*/
if (file->entry != 0)
	{
	puts("APPLICATION");
	puts("-----------");
	application_start = raw_file + boot_data->start - file->self;
	application_end = application_start + boot_data->length;
	if (application_end > raw_file_end)
		application_end = raw_file_end;
	dump_buffer((unsigned char *)application_start, boot_data->start, application_end - application_start);
	}

return 0;
}

/*
	READ_ENTIRE_FILE()
	------------------
*/
char *read_entire_file(char *filename, long long *file_length)
{
long long unused;
char *block = NULL;
FILE *fp;
struct stat details;

if (filename == NULL)
	return NULL;

if (file_length == NULL)
	file_length = &unused;

if ((fp = fopen(filename, "rb")) == NULL)
	return NULL;

if (fstat(_fileno(fp), &details) == 0)
	if ((*file_length = details.st_size) != 0)
		if ((block = (char *)malloc((size_t)(details.st_size + 1))) != NULL)		// +1 for the '\0' on the end
			if (fread(block, details.st_size, 1, fp) == 1)
				block[details.st_size] = '\0';
			else
				{
				free(block);
				block = NULL;
				}
fclose(fp);

return block;
}

/*
   USAGE()
   ------
*/
int usage(char *exename)
{
return printf("Usage: %s <imx_filename>\n", exename);
}

/*
   MAIN()
   ------
*/
int main(int argc, char *argv[])
{
char *file;
long long size;

if (argc != 2)
   exit(usage(argv[0]));

if ((file = read_entire_file(argv[1], &size)) == NULL)
   exit(printf("Can't open and read input file:%s\n", argv[1]));

return imx_image_process(file, (uint32_t)size);
}
