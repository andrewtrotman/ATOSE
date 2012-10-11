/*
	BIN_TO_C.C
	----------
	Turn any file into C source code
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
	READ_ENTIRE_FILE()
	------------------
*/
unsigned char *read_entire_file(char *filename, long *size)
{
FILE *fp;
struct stat stats;
unsigned char *buffer;

if (stat(filename, &stats) != 0)
	return NULL;

if ((fp = fopen(filename, "rb")) == NULL)
	return NULL;

buffer = (unsigned char *)malloc(stats.st_size + 1);
if (fread(buffer, stats.st_size, 1, fp) != 1)
	{
	free(buffer);
	buffer = NULL;
	}
else
	buffer[stats.st_size] = '\0';

fclose(fp);

if (size != NULL)
	*size = stats.st_size;

return buffer;
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
unsigned char *file;
FILE *outfile;
long current, size;

if (argc != 4)
	exit(printf("Usage:%s <input_bin_file> <outfile> <c_identidier>\n", argv[0]));
if ((file = read_entire_file(argv[1], &size)) == NULL)
	exit(printf("Can't open file:%s\n", argv[1]));
if ((outfile = fopen(argv[2], "wb")) == NULL)
	exit(printf("Can't open file:%s\n", argv[1]));

fprintf(outfile, "/*\nGenerated by:%s %s %s %s\n*/\n", argv[0], argv[1], argv[2], argv[3]);
fprintf(outfile, "unsigned char %s[]={", argv[3]);

for (current = 0; current < size; current++)
	if (current == 0x0FFF)
		fprintf(outfile, "0x%02X", (unsigned)file[current]);
	else
		{
		if (current % 8 == 0)
			fprintf(outfile, "\n");
		fprintf(outfile, "0x%02X, ", (unsigned)file[current]);
		}

fprintf(outfile, "\n};\n");

fprintf(outfile, "uint32_t %s_size = %lld;\n", argv[3], (long long)size);

fclose(outfile);

return 0;
}
