/*
   IMAGE_DUMP
   ==========
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
   IMX_IMAGE_IVT_HEADER
   --------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
uint8_t tag;
uint16_t length;
uint8_t version;
} imx_image_ivt_header;
#pragma pack()

/*
   IMX_IMAGE_FLASH_HEADER
   ----------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
imx_image_ivt_header header;
uint32_t entry;
uint32_t reserved1;
uint32_t dcd_ptr;
uint32_t boot_data_ptr;
uint32_t self;
uint32_t csf;
uint32_t reserved2;
} imx_image_flash_header;
#pragma pack()

/*
   IMX_IMAGE_BOOT_DATA
   -------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
uint32_t start;
uint32_t size;
uint32_t plugin;
} imx_image_boot_data;
#pragma pack()

/*
   IMX_IMAGE_WRITE_DCD_COMMAND
   ---------------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
uint8_t tag;
uint16_t length;
uint8_t param;
} imx_image_write_dcd_command;
#pragma pack()

/*
   IMX_IMAGE_DCD_ADDR_DATA
   -----------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
uint32_t addr;
uint32_t value;
} imx_image_dcd_addr_data;
#pragma pack()

/*
   IMX_IMAGE_DCD
   -------------
*/
#define MAX_HW_CFG_SIZE_V2 121
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
imx_image_ivt_header header;
imx_image_write_dcd_command write_dcd_command;
imx_image_dcd_addr_data addr_data[MAX_HW_CFG_SIZE_V2];
} imx_image_dcd;
#pragma pack()

/*
   IMX_IMAGE_HEADER
   ----------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
imx_image_flash_header fhdr;
imx_image_boot_data boot_data;
imx_image_dcd dcd_table;
uint32_t flash_offset;
} imx_image_header;
#pragma pack()

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
FILE *fp;
imx_image_header header;

if (argc != 2)
   exit(usage(argv[0]));

if ((fp = fopen(argv[1], "rb")) == NULL)
   exit(printf("Can't open input file:%s\n", argv[1]));

fread(&header, sizeof(header), 1, fp);

fclose(fp);

return 0;
}
