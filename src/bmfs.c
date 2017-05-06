/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

/* Global includes */
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "bmfs.h"
#include "stdlib.h"

char s_list[] = "list";
char s_format[] = "format";
char s_initialize[] = "initialize";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
char s_delete[] = "delete";
char s_version[] = "version";

static int format_file(struct BMFSDisk *disk, long bytes);

static void list_entries(struct BMFSDisk *disk);

static void print_usage(const char *argv0);

static void print_version(void);

/* Program code */
int main(int argc, char *argv[])
{
	struct BMFSDisk disk;
	FILE *diskfile;
	char *diskname;
	char *command;
	char *filename;
	char tempstring[32];
	unsigned int filesize;

	/* Parse arguments */
	if (argc < 3)
	{
		if (argc > 1)
		{
			if (strcasecmp(s_version, argv[1]) == 0)
				print_version();
		}
		else
		{
			print_usage(argv[0]);
		}
		/* the program wasn't called
		 * properly, should be considered
		 * a program failure */
		return EXIT_FAILURE;
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
			int ret = bmfs_initialize(diskname, size, mbr, boot, kernel);
			if (ret != 0)
				return EXIT_FAILURE;
			else
				return EXIT_SUCCESS;
		}
		else
		{
			printf("Usage: %s disk %s ", argv[0], command);
			printf("size [mbr_file] ");
			printf("[bootloader_file] [kernel_file]\n");
			return EXIT_FAILURE;
		}
	}

	if ((diskfile = fopen(diskname, "r+b")) == NULL)			// Open for read/write in binary mode
	{
		printf("Error: Unable to open disk '%s'\n", diskname);
		return EXIT_FAILURE;
	}

	bmfs_disk_init_file(&disk, diskfile);

	/* Opened ok, is it a valid BMFS disk? */
	if (bmfs_disk_check_tag(&disk) != 0)
	{
		if (strcasecmp(s_format, command) == 0)
		{
			format_file(&disk, BMFS_MINIMUM_DISK_SIZE);
			fclose(diskfile);
			return EXIT_SUCCESS;
		}
		printf("Error: Not a valid BMFS drive (Disk is not BMFS formatted).\n");
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	if (strcasecmp(s_list, command) == 0)
	{
		list_entries(&disk);
	}
	else if (strcasecmp(s_format, command) == 0)
	{
		if (argc > 3)
		{
			if (strcasecmp(argv[3], "/FORCE") == 0)
			{
				format_file(&disk, BMFS_MINIMUM_DISK_SIZE);
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
				if (filesize < 1)
				{
					printf("Error: Invalid file size.\n");
					return EXIT_FAILURE;
				}

				int err = bmfs_disk_create_file(&disk, filename, filesize);
				if (err != 0)
				{
					fprintf(stderr, "%s: Failed to create '%s'\n", argv[0], filename);
					fprintf(stderr, "  %s\n", strerror(-err));
					return EXIT_FAILURE;
				}
			}
			else
			{
				printf("Maximum file size in MiB: ");
				if (fgets(tempstring, sizeof(tempstring), stdin) != NULL)	// Get up to 32 chars from the keyboard
					filesize = atoi(tempstring);
				else
					return EXIT_FAILURE;

				if (filesize < 1)
				{
					printf("Error: Invalid file size.\n");
					return EXIT_FAILURE;
				}

				int err = bmfs_disk_create_file(&disk, filename, filesize);
				if (err != 0)
				{
					fprintf(stderr, "%s: Failed to create '%s'\n", argv[0], filename);
					fprintf(stderr, "  %s\n", strerror(-err));
					return EXIT_FAILURE;
				}
			}
		}
	}
	else if (strcasecmp(s_read, command) == 0)
	{
		bmfs_readfile(&disk, filename);
	}
	else if (strcasecmp(s_write, command) == 0)
	{
		bmfs_writefile(&disk, filename);
	}
	else if (strcasecmp(s_delete, command) == 0)
	{
		bmfs_disk_delete_file(&disk, filename);
	}
	else
	{
		printf("Error: Unknown command\n");
	}
	if (diskfile != NULL)
	{
		fclose(diskfile);
	}
	return EXIT_SUCCESS;
}


int format_file(struct BMFSDisk *disk, long bytes)
{
	int err = bmfs_disk_seek(disk, bytes - 1, SEEK_SET);
	if (err != 0)
		return err;

	if (fputc(0, (FILE*)(disk->disk)) != 0)
		return -errno;

	err = bmfs_disk_format(disk);
	if (err != 0)
		return err;

	return 0;
}

static void list_entries(struct BMFSDisk *disk)
{
	struct BMFSDir dir;
	int err = bmfs_disk_read_dir(disk, &dir);
	if (err != 0)
		return;

	printf("| Name                             |             Size (B) |       Reserved (MiB) |\n");
	printf("|----------------------------------|----------------------|----------------------|\n");
	for (size_t i = 0; i < 64; i++)
	{
		const struct BMFSEntry *entry;
		entry = &dir.Entries[i];
		if (bmfs_entry_is_empty(entry))
			continue;
		else if (bmfs_entry_is_terminator(entry))
			break;
		else
			printf("| %-32s | %20llu | %20llu |\n",
			       entry->FileName,
			       (unsigned long long)(entry->FileSize),
			       (unsigned long long)(entry->ReservedBlocks * 2));
	}
}

static void print_usage(const char *argv0)
{
	printf("Usage: %s disk function [file]\n", argv0);
	printf("\n");
	printf("Disk: the name of the disk file\n");
	printf("\n");
	printf("Functions:\n");
	printf("\tlist   : lists entries in the BMFS file system\n");
	printf("\tread   : reads a file from the BMFS file system to the host file system\n");
	printf("\twrite  : writes a file from the host file system to BMFS file system\n");
	printf("\tcreate : creates a file within a BMFS file system\n");
	printf("\tdelete : deletes a file within a BMFS file system\n");
	printf("\tformat : formats an existing file with BMFS\n");
	printf("\tinitialize : creates an image for the BareMetal operating system\n");
	printf("\n");
	printf("File: may be used in a read, write, create or delete operation\n");
}

static void print_version(void)
{
	printf("BareMetal File System Utility v1.2.3 (2017 04 07)\n");
	printf("Written by Ian Seyler @ Return Infinity (ian.seyler@returninfinity.com)\n");
}

/* EOF */
