/* BareMetal File System Lite Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.0 (2024 11 21) */

/* Global includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

/* Typedefs */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Global defines */
struct BMFSEntry
{
	char FileName[32];
	u64 StartingBlock; // Files start at block 4
	u64 ReservedBlocks;
	u64 FileSize;
	u64 Unused;
};

/* Global constants */
// Min drive size is 64KiB
const unsigned int minimumDiskSize = (64 * 1024);
// Max drive size is 2MiB
const unsigned int maximumDiskSize = (2 * 1024 * 1024);
// Block size in BMFS-Lite is 1KiB
const unsigned int blockSize = 1024;

/* Global variables */
FILE *file, *disk;
unsigned int filesize, disksize, retval;
char tempfilename[32], tempstring[32];
char *filename, *diskname, *command;
char s_list[] = "list";
char s_format[] = "format";
char s_initialize[] = "initialize";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
struct BMFSEntry entry;
void *pentry = &entry;
char *BlockMap;
char *FileBlocks;
char Directory[4096];

/* Built-in functions */
int bmfs_find(char *filename, struct BMFSEntry *fileentry, int *entrynumber);
void bmfs_list(void);
void bmfs_format(void);
int bmfs_initialize(char *diskname, char *size);
void bmfs_create(char *filename, unsigned long long maxsize);
void bmfs_read(char *filename);
void bmfs_write(char *filename);

/* Program code */
int main(int argc, char *argv[])
{
	/* Parse arguments */
	if (argc == 1) // No arguments provided
	{
		printf("BareMetal File System Lite Utility v1.0 (2024 11 19)\n");
		printf("Written by Ian Seyler @ Return Infinity (ian.seyler@returninfinity.com)\n\n");
		printf("Usage: bmfs disk function file\n\n");
		printf("Disk:     the name of the disk file\n");
		printf("Function: list, read, write, create, format, initialize\n");
		printf("File:     (if applicable)\n");
		exit(EXIT_SUCCESS);
	}

	if (argc >= 2)
	{
		diskname = (argc > 1 ? argv[1] : NULL);
		command = (argc > 2 ? argv[2] : NULL);
		filename = (argc > 3 ? argv[3] : NULL);
	}

	if (argc > 2 && strcasecmp(s_initialize, command) == 0)
	{
		if (argc >= 4)
		{
			// Create a disk image of the requested size
			char *size = (argc > 3 ? argv[3] : NULL);	// Required
			int ret = bmfs_initialize(diskname, size);
			exit(ret);
		}
		else
		{
			// Create a 1MiB disk image if no size was requested
			char *size = "1048576";
			int ret = bmfs_initialize(diskname, size);
			exit(ret);
		}
	}

	if ((disk = fopen(diskname, "r+b")) == NULL)			// Open for read/write in binary mode
	{
		printf("bmfs error: Unable to open disk '%s'\n", diskname);
		exit(EXIT_FAILURE);
	}
	else								// Opened ok, is it a valid BMFS disk?
	{
		fseek(disk, 0, SEEK_END);
		disksize = ftell(disk);					// Disk size in Bytes
		fseek(disk, 0, SEEK_SET);				// Seek to start for directory
		retval = fread(Directory, 4096, 1, disk);		// Read 4096 bytes to the Directory buffer
		rewind(disk);
	}

	if (strcasecmp(s_list, command) == 0)
	{
		bmfs_list();
	}
	else if (strcasecmp(s_format, command) == 0)
	{
		if (argc > 3)
		{
			if (strcasecmp(argv[3], "/FORCE") == 0)
			{
				bmfs_format();
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
			printf("bmfs-lite error: File name not specified.\n");
		}
		else
		{
			if (argc > 4)
			{
				int filesize = atoi(argv[4]);
				if (filesize >= 1)
				{
					bmfs_create(filename, filesize);
				}
				else
				{
					printf("bmfs-lite error: Invalid file size.\n");
				}
			}
			else
			{
				printf("Maximum file size in bytes: ");
				if (fgets(tempstring, 32, stdin) != NULL)	// Get up to 32 chars from the keyboard
					filesize = atoi(tempstring);
				if (filesize >= 1)
					bmfs_create(filename, filesize);
				else
					printf("bmfs-lite error: Invalid file size.\n");
			}
		}
	}
	else if (strcasecmp(s_read, command) == 0)
	{
		bmfs_read(filename);
	}
	else if (strcasecmp(s_write, command) == 0)
	{
		bmfs_write(filename);
	}
	else
	{
		printf("bmfs error: Unknown command\n");
	}

	if (disk != NULL)
	{
		fclose( disk );
		disk = NULL;
	}

	return 0;
}


int bmfs_find(char *filename, struct BMFSEntry *fileentry, int *entrynumber)
{
	int tint;

	for (tint = 0; tint < 64; tint++)
	{
		memcpy(pentry, Directory+(tint*64), 64);
		if (entry.FileName[0] == 0x00)				// End of directory
		{
			tint = 64;
		}
		else if (entry.FileName[0] == 0x01)			// Empty entry
		{
			// Ignore
		}
		else							// Valid entry
		{
			if (strcmp(filename, entry.FileName) == 0)
			{
				memcpy(fileentry, pentry, 64);
				*entrynumber = tint;
				return 1;
			}
		}
	}
	return 0;
}


void bmfs_list(void)
{
	int tint;

	printf("BMFS-Lite Drive Size: %d bytes\n", disksize);
	printf("Name                            |    Size | Reserved | Block\n");
	printf("============================================================\n");
	for (tint = 0; tint < 64; tint++)				// Max 64 entries
	{
		memcpy(pentry, Directory+(tint*64), 64);
		if (entry.FileName[0] == 0x00)				// End of directory, bail out
		{
			tint = 64;
		}
		else if (entry.FileName[0] == 0x01)			// Empty entry
		{
			// Ignore
		}
		else							// Valid entry
		{
			printf("%-32s %8lld %10lld %7lld\n", entry.FileName, (long long int)entry.FileSize, (long long int)(entry.ReservedBlocks*blockSize), (long long int)entry.StartingBlock);
		}
	}
}


void bmfs_format(void)
{
// Todo - overwrite entire file with zeros
}


int bmfs_initialize(char *diskname, char *size)
{
	unsigned long long diskSize = 0;
	unsigned long long writeSize = 0;
	size_t bufferSize = 50 * 1024;
	char * buffer = NULL;
	int diskSizeFactor = 0;
	size_t chunkSize = 0;
	int ret = 0;
	size_t i;

	// Validate the disk size string and convert it to an integer value.
	for (i = 0; size[i] != '\0' && ret == 0; ++i)
	{
		char ch = size[i];
		if (isdigit(ch))
		{
			unsigned int n = ch - '0';
			if (diskSize * 10 > diskSize ) // Make sure we don't overflow
			{
				diskSize *= 10;
				diskSize += n;
			}
			else if (diskSize == 0) // First loop iteration
			{
				diskSize += n;
			}
			else
			{
				printf("bmfs error: Disk size is too large\n");
				ret = 1;
			}
		}
		else if (i == 0) // No digits specified
		{
			printf("bmfs error: A numeric disk size must be specified\n");
			ret = 1;
		}
		else
		{
			switch (toupper(ch))
			{
				case 'K':
					diskSizeFactor = 1;
					break;
				case 'M':
					diskSizeFactor = 2;
					break;
				default:
					printf("bmfs error: Invalid disk size string: '%s'\n", size);
					ret = 1;
					break;
			}

			// If this character is a valid unit indicator, but is not at the
			// end of the string, then the string is invalid.
			if (ret == 0 && size[i+1] != '\0')
			{
				printf("bmfs error: Invalid disk size string: '%s'\n", size);
				ret = 1;
			}
		}
	}

	// Adjust the disk size if a unit indicator was given.  Note that an
	// input of something like "0" or "0K" will get past the checks above.
	if (ret == 0 && diskSize > 0 && diskSizeFactor > 0)
	{
		while (diskSizeFactor--)
		{
			if (diskSize * 1024 > diskSize ) // Make sure we don't overflow
			{
				diskSize *= 1024;
			}
			else
			{
				printf("bmfs error: Disk size is too large\n");
				ret = 1;
			}
		}
	}

	// Make sure the disk size is large enough.
	if (ret == 0)
	{
		if (diskSize < minimumDiskSize)
		{
			printf("bmfs error: Disk size must be at least %d bytes\n", minimumDiskSize);
			ret = 1;
		}
		if (diskSize > maximumDiskSize)
		{
			printf("bmfs error: Disk size must not be greater than %d bytes\n", maximumDiskSize);
			ret = 1;
		}
	}

	// Allocate buffer to use for filling the disk image with zeros.
	if (ret == 0)
	{
		buffer = (char *) malloc(bufferSize);
		if (buffer == NULL)
		{
			printf("bmfs error: Failed to allocate buffer\n");
			ret = 1;
		}
	}

	// Open the disk image file for writing.  This will truncate the disk file
	// if it already exists, so we should do this only after we're ready to
	// actually write to the file.
	if (ret == 0)
	{
		disk = fopen(diskname, "wb");
		if (disk == NULL)
		{
			printf("bmfs error: Unable to open disk '%s'\n", diskname);
			ret = 1;
		}
	}

	// Fill the disk image with zeros.
	if (ret == 0)
	{
		// double percent;
		memset(buffer, 0, bufferSize);
		writeSize = 0;
		while (writeSize < diskSize)
		{
			// percent = writeSize;
			// percent /= diskSize;
			// percent *= 100;
			// printf("Formatting disk: %llu of %llu bytes (%.0f%%)...\r", writeSize, diskSize, percent);
			chunkSize = bufferSize;
			if (chunkSize > diskSize - writeSize)
			{
				chunkSize = diskSize - writeSize;
			}
			if (fwrite(buffer, chunkSize, 1, disk) != 1)
			{
				printf("bmfs error: Failed to write disk '%s'\n", diskname);
				ret = 1;
				break;
			}
			writeSize += chunkSize;
		}
		if (ret == 0)
		{
			// printf("Formatting disk: %llu of %llu bytes (100%%)%9s\n", writeSize, diskSize, "");
		}
	}

	// Format the disk.
	if (ret == 0)
	{
		rewind(disk);
		bmfs_format();
	}

	if (disk != NULL)
	{
		fclose(disk);
		disk = NULL;
	}

	// Free the buffer if it was allocated.
	if (buffer != NULL)
	{
		free(buffer);
	}

	if (ret == 0)
	{
		// printf("Disk initialization complete.\n");
	}

	return ret;
}

// helper function for qsort, sorts by StartingBlock field
static int StartingBlockCmp(const void *pa, const void *pb)
{
	struct BMFSEntry *ea = (struct BMFSEntry *)pa;
	struct BMFSEntry *eb = (struct BMFSEntry *)pb;
	// empty records go to the end
	if (ea->FileName[0] == 0x01)
		return 1;
	if (eb->FileName[0] == 0x01)
		return -1;
	// compare non-empty records by their starting blocks number
	return (ea->StartingBlock - eb->StartingBlock);
}

void bmfs_create(char *filename, unsigned long long maxsize)
{
	struct BMFSEntry tempentry;
	int slot;

	if (bmfs_find(filename, &tempentry, &slot) == 0)
	{
		unsigned long long blocks_requested = maxsize / 2; // how many blocks to allocate
		char dir_copy[4096]; // copy of directory
		int num_used_entries = 0; // how many entries of Directory are either used or deleted
		int first_free_entry = -1; // where to put new entry
		int tint;
		struct BMFSEntry *pEntry;
		unsigned long long new_file_start = 0;

		if(strlen(filename) > 31)
		{
			printf("bmfs error: Filename too long.\n");
			return;
		}

		// Make a copy of Directory to play with
		memcpy(dir_copy, Directory, 4096);

		// Calculate number of files
		for (tint = 0; tint < 64; tint++)
		{
			pEntry = (struct BMFSEntry *)(dir_copy + tint * 64); // points to the current directory entry
			if (pEntry->FileName[0] == 0x00) // end of directory
			{
				num_used_entries = tint;
				if (first_free_entry == -1)
					first_free_entry = tint; // there were no unused entires before, will use this one
				break;
			}
			else if (pEntry->FileName[0] == 0x01) // unused entry
			{
				if (first_free_entry == -1)
					first_free_entry = tint; // will use it for our new file
			}
		}

		if (first_free_entry == -1)
		{
			printf("bmfs error: Cannot create file. No free directory entries.\n");
			return;
		}

		// Find an area with enough free blocks
		// Sort our copy of the directory by starting block number
		qsort(dir_copy, num_used_entries, 64, StartingBlockCmp);

		for (tint = 0; tint < num_used_entries + 1; tint++)
		{
			// on each iteration of this loop we'll see if a new file can fit
			// between the end of the previous file (initially == 1)
			// and the beginning of the current file (or the last data block if there are no more files).

			pEntry = (struct BMFSEntry *)(dir_copy + tint * 64); // points to the current directory entry

			new_file_start += pEntry->ReservedBlocks;
		}
		new_file_start += 4; // Skip the directory

		// Add file record to Directory
		pEntry = (struct BMFSEntry *)(Directory + first_free_entry * 64);
		pEntry->StartingBlock = new_file_start;
		pEntry->ReservedBlocks = blocks_requested;
		pEntry->FileSize = 0;
		strcpy(pEntry->FileName, filename);

		if (first_free_entry == num_used_entries && num_used_entries + 1 < 64)
		{
			// here we used the record that was marked with 0x00,
			// so make sure to mark the next record with 0x00 if it exists
			pEntry = (struct BMFSEntry *)(Directory + (num_used_entries + 1) * 64);
			pEntry->FileName[0] = 0x00;
		}

		// Flush Directory to disk
		fseek(disk, 0, SEEK_SET);				// Seek to start for directory
		fwrite(Directory, 4096, 1, disk);			// Write 4096 bytes for the Directory

		// printf("Complete: file %s starts at block %lld, directory entry #%d.\n", pEntry->FileName, pEntry->StartingBlock, first_free_entry);
	}
	else
	{
		printf("bmfs error: File already exists.\n");
	}
}

// Read a file from a BMFS volume
void bmfs_read(char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int slot, retval;
	unsigned long long bytestoread;
	char *buffer;

	if (0 == bmfs_find(filename, &tempentry, &slot))
	{
		printf("bmfs error: File not found in BMFS.\n");
	}
	else
	{
		if ((tfile = fopen(tempentry.FileName, "wb")) == NULL)
		{
			printf("bmfs error: Could not open local file '%s'\n", tempentry.FileName);
		}
		else
		{
			bytestoread = tempentry.FileSize;
			fseek(disk, tempentry.StartingBlock*blockSize, SEEK_SET); // Skip to the starting block in the disk
			buffer = malloc(blockSize);
			if (buffer == NULL)
			{
				printf("bmfs error: Unable to allocate enough memory for buffer.\n");
			}
			else
			{
				while (bytestoread != 0)
				{
					if (bytestoread >= blockSize)
					{
						retval = fread(buffer, blockSize, 1, disk);
						if (retval == 1)
						{
							fwrite(buffer, blockSize, 1, tfile);
							bytestoread -= blockSize;
						}
						else
						{
							printf("bmfs error: Unexpected read length detected.\n");
							bytestoread = 0;
						}
					}
					else
					{
						retval = fread(buffer, bytestoread, 1, disk);
						if (retval == 1)
						{
							fwrite(buffer, bytestoread, 1, tfile);
							bytestoread = 0;
						}
						else
						{
							printf("bmfs error: Unexpected read length detected.\n");
							bytestoread = 0;
						}
					}
				}
			}
			fclose(tfile);
		}
	}
}


// Write a file to a BMFS volume
void bmfs_write(char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int slot, retval;
	unsigned long long tempfilesize;
	char *buffer;

	if ((tfile = fopen(filename, "rb")) == NULL)
	{
		printf("bmfs error: Could not open local file '%s'\n", filename);
	}
	else
	{
		// Is there enough room in BMFS?
		fseek(tfile, 0, SEEK_END);
		tempfilesize = ftell(tfile);
		rewind(tfile);
		if (0 == bmfs_find(filename, &tempentry, &slot))
		{
			bmfs_create(filename, (tempfilesize+blockSize)/blockSize*2);
			bmfs_find(filename, &tempentry, &slot);
		}
		if ((tempentry.ReservedBlocks*blockSize) < tempfilesize)
		{
			printf("bmfs error: Not enough reserved space in BMFS.\n");
		}
		else
		{
			fseek(disk, tempentry.StartingBlock*blockSize, SEEK_SET); // Skip to the starting block in the disk
			buffer = malloc(blockSize);
			if (buffer == NULL)
			{
				printf("bmfs error: Unable to allocate enough memory for buffer.\n");
			}
			else
			{
				while (tempfilesize != 0)
				{
					if (tempfilesize >= blockSize)
					{
						retval = fread(buffer, blockSize, 1, tfile);
						if (retval == 1)
						{
							fwrite(buffer, blockSize, 1, disk);
							tempfilesize -= blockSize;
						}
						else
						{
							printf("bmfs error: Unexpected read length detected.\n");
							tempfilesize = 0;
						}
					}
					else
					{
						retval = fread(buffer, tempfilesize, 1, tfile);
						if (retval == 1)
						{
							memset(buffer+(tempfilesize), 0, (blockSize-tempfilesize)); // 0 the rest of the buffer
							fwrite(buffer, blockSize, 1, disk);
							tempfilesize = 0;
						}
						else
						{
							printf("bmfs error: Unexpected read length detected.\n");
							tempfilesize = 0;
						}
					}
				}
			}
			// Update directory
			tempfilesize = ftell(tfile);
			memcpy(Directory+(slot*64)+48, &tempfilesize, 8);
			fseek(disk, 0, SEEK_SET);			// Seek to directory
			fwrite(Directory, 4096, 1, disk);		// Write new directory to disk
		}
		fclose(tfile);
	}
}


/* EOF */
