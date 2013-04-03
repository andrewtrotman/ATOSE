/*
	CLIENT_FILE.H
	-------------
	implementation of the fopen() routines for ATOSE
*/
#ifndef CLIENT_FILE_H_
#define CLIENT_FILE_H_

#include <stdint.h>

class ATOSE_FILE;

#define ATOSE_SEEK_SET 0
#define ATOSE_SEEK_CUR 1
//#define ATOSE_SEEK_END 2

ATOSE_FILE *ATOSE_fopen(const char *path, const char *mode);
int32_t ATOSE_fclose(ATOSE_FILE *stream);
int32_t ATOSE_fseek(ATOSE_FILE *stream, int64_t offset, uint8_t from);
uint64_t ATOSE_ftell(ATOSE_FILE *stream);
uint32_t ATOSE_fread(void *destination, uint64_t size, uint32_t count, ATOSE_FILE *stream);
//uint64_t ATOSE_fwrite(const void *source, uint64_t size, uint64_t count, ATOSE_FILE *stream);

#endif
