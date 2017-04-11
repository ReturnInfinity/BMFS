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
#include "libbmfs.h"


char s_list[] = "list";
char s_format[] = "format";
char s_initialize[] = "initialize";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
char s_delete[] = "delete";
char s_version[] = "version";


static int format_file(FILE *diskfile, long bytes);

static void list_entries(FILE *diskfile);

/* Program code */
int main(int argc, char *argv[])
{
	FILE *disk;
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
			int ret = bmfs_initialize(diskname, size, mbr, boot, kernel);
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
		struct BMFSDisk tmp_disk;
		bmfs_disk_init_file(&tmp_disk, disk);
		if (bmfs_disk_check_tag(&tmp_disk) != 0)			// Is it a BMFS formatted disk?
		{
			if (strcasecmp(s_format, command) == 0)
			{
				format_file(disk, minimumDiskSize);
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
		list_entries(disk);
	}
	else if (strcasecmp(s_format, command) == 0)
	{
		if (argc > 3)
		{
			if (strcasecmp(argv[3], "/FORCE") == 0)
			{
				format_file(disk, minimumDiskSize);
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

				struct BMFSDisk tmp_disk;
				bmfs_disk_init_file(&tmp_disk, disk);
				int err = bmfs_disk_create_file(&tmp_disk, filename, filesize);
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
				if (filesize < 1)
				{
					printf("Error: Invalid file size.\n");
					return EXIT_FAILURE;
				}

				struct BMFSDisk tmp_disk;
				bmfs_disk_init_file(&tmp_disk, disk);
				int err = bmfs_disk_create_file(&tmp_disk, filename, filesize);
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
		struct BMFSDisk tmp_disk;
		bmfs_disk_init_file(&tmp_disk, disk);
		bmfs_readfile(&tmp_disk, filename);
	}
	else if (strcasecmp(s_write, command) == 0)
	{
		struct BMFSDisk tmp_disk;
		bmfs_disk_init_file(&tmp_disk, disk);
		bmfs_writefile(&tmp_disk, filename);
	}
	else if (strcasecmp(s_delete, command) == 0)
	{
		struct BMFSDisk tmp_disk;
		bmfs_disk_init_file(&tmp_disk, disk);
		bmfs_disk_delete_file(&tmp_disk, filename);
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


int format_file(FILE *file, long bytes)
{
	if (fseek(file, bytes - 1, SEEK_SET) != 0)
		return -errno;

	if (fputc(0, file) != 0)
		return -errno;

	struct BMFSDisk disk;
	int err = bmfs_disk_init_file(&disk, file);
	if (err != 0)
		return err;

	err = bmfs_disk_format(&disk);
	if (err != 0)
		return err;

	return 0;
}

static void list_entries(FILE *diskfile)
{
	struct BMFSDisk disk;
	int err = bmfs_disk_init_file(&disk, diskfile);
	if (err != 0)
		return;

	struct BMFSDir dir;
	err = bmfs_disk_read_dir(&disk, &dir);
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


/* EOF */
