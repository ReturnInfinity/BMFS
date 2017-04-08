/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

/* Global includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "libbmfs.h"


char s_list[] = "list";
char s_format[] = "format";
char s_initialize[] = "initialize";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
char s_delete[] = "delete";
char s_version[] = "version";


/* Program code */
int main(int argc, char *argv[])
{
	/* Parse arguments */
	if (argc < 3)
	{
		if (argc > 1)
		{
			if (strcasecmp(s_version, argv[1]) == 0)
			{
				printf("BareMetal File System Utility v1.2.3 (2017 04 07)\n");
				printf("Written by Ian Seyler @ Return Infinity (ian.seyler@returninfinity.com)\n");
			}
		}
		else
		{
			printf("Usage: %s disk function file\n", argv[0]);
			printf("\tDisk: the name of the disk file\n");
			printf("\tFunction: list, read, write, create, delete, format, initialize\n");
			printf("\tFile: (if applicable)\n");
		}
		exit(0);
	}

	diskname = argv[1];
	command = argv[2];
	filename = argv[3];

	if (strcasecmp(s_initialize, command) == 0)
	{
		if (argc >= 4)
		{
			char *size = argv[3];				// Required
			char *mbr = (argc > 4 ? argv[4] : NULL);    	// Opt.
			char *boot = (argc > 5 ? argv[5] : NULL);   	// Opt.
			char *kernel = (argc > 6 ? argv[6] : NULL); 	// Opt.
			int ret = initialize(diskname, size, mbr, boot, kernel);
			exit(ret);
		}
		else
		{
			printf("Usage: %s disk %s ", argv[0], command);
			printf("size [mbr_file] ");
			printf("[bootloader_file] [kernel_file]\n");
			exit(1);
		}
	}

	if ((disk = fopen(diskname, "r+b")) == NULL)			// Open for read/write in binary mode
	{
		printf("Error: Unable to open disk '%s'\n", diskname);
		exit(0);
	}
	else								// Opened ok, is it a valid BMFS disk?
	{
		fseek(disk, 0, SEEK_END);
		disksize = ftell(disk) / 1048576;			// Disk size in MiB
		fseek(disk, 1024, SEEK_SET);				// Seek 1KiB in for disk information
		retval = fread(DiskInfo, 512, 1, disk);			// Read 512 bytes to the DiskInfo buffer
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		retval = fread(Directory, 4096, 1, disk);		// Read 4096 bytes to the Directory buffer
		rewind(disk);

		if (strcasecmp(DiskInfo, fs_tag) != 0)			// Is it a BMFS formatted disk?
		{
			if (strcasecmp(s_format, command) == 0)
			{
				format();
			}
			else
			{
				printf("Error: Not a valid BMFS drive (Disk is not BMFS formatted).\n");
			}
			fclose(disk);
			return 0;
		}
	}

	if (strcasecmp(s_list, command) == 0)
	{
		list();
	}
	else if (strcasecmp(s_format, command) == 0)
	{
		if (argc > 3)
		{
			if (strcasecmp(argv[3], "/FORCE") == 0)
			{
				format();
			}
			else
			{
				printf("Format aborted!\n");
			}
		}
		else
		{
			printf("Format aborted!\n");
		}
	}
	else if (strcasecmp(s_create, command) == 0)
	{
		if (filename == NULL)
		{
			printf("Error: File name not specified.\n");
		}
		else
		{
			if (argc > 4)
			{
				int filesize = atoi(argv[4]);
				if (filesize >= 1)
				{
					create(filename, filesize);
				}
				else
				{
			  		printf("Error: Invalid file size.\n");
				}
			}
			else
			{
				printf("Maximum file size in MiB: ");
				if (fgets(tempstring, 32, stdin) != NULL)	// Get up to 32 chars from the keyboard
					filesize = atoi(tempstring);
				if (filesize >= 1)
					create(filename, filesize);
				else
					printf("Error: Invalid file size.\n");
			}
		}
	}
	else if (strcasecmp(s_read, command) == 0)
	{
		read(filename);
	}
	else if (strcasecmp(s_write, command) == 0)
	{
		write(filename);
	}
	else if (strcasecmp(s_delete, command) == 0)
	{
		delete(filename);
	}
	else
	{
		printf("Error: Unknown command\n");
	}
	if (disk != NULL)
	{
		fclose( disk );
		disk = NULL;
	}
	return 0;
}


/* EOF */
