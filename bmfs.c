/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */


/* Global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global defines */
struct BMFSEntry
{
	unsigned char FileName[32];
	unsigned long long StartingBlock;
	unsigned long long ReservedBlocks;
	unsigned long long FileSize;
	unsigned long long Unused;
};
#define WHERE fprintf(stderr,"[LOG]%s:%d\n",__FILE__,__LINE__);

/* Global variables */
FILE *file, *disk;
unsigned int filesize, disksize;
char tempfilename[32], tempstring[32];
char *filename, *diskname, *command;
char s_list[] = "list";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
char s_delete[] = "delete";
struct BMFSEntry entry;
void *pentry = &entry;
unsigned char Directory[4096];

/* Built-in functions */
int findfile(char *filename, struct BMFSEntry *fileentry, int *entrynumber);
void list();
void create(char *filename, unsigned long long maxsize);
void read(char *filename);
void write(char *filename);
void delete(char *filename);

/* Program code */
int main(int argc, char *argv[])
{
	/* first argument needs to be a NIC */
	if (argc < 3)
	{
		printf("BareMetal File System Utility v0.1 (2012 12 29)\n");
		printf("Written by Ian Seyler @ Return Infinity (ian.seyler@returninfinity.com)\n\n");
		printf("Usage: %s disk function file\n", argv[0]);
		printf("Disk: the name of the disk file\n");
		printf("Function: list, read, write, create, delete\n");
		printf("File: (if applicable)\n");
		exit(0);
	}

	diskname = argv[1];
	command = argv[2];
	filename = argv[3];

	if ((disk = fopen(diskname, "r+b")) == NULL)	// Open for read/write in binary mode
	{
		printf("Error opening disk '%s'\n", diskname);
	}
	else
	{
		fseek(disk, 0, SEEK_END);
		disksize = ftell(disk) / 1048576;			// Disk size in MiB
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		fread(Directory, 4096, 1, disk);			// Read 4096 bytes to the Directory buffer
		rewind(disk);

		if (strcasecmp(s_list, command) == 0)
		{
			list();
		}

		if (argc < 4)
		{
			printf("Insufficient arguments.\n");
		}
		else
		{
			if (strcasecmp(s_create, command) == 0)
			{
				printf("Maximum file size in MiB: ");
				fgets(tempstring, 32, stdin);			// Get up to 32 chars from the keyboard
				filesize = atoi(tempstring);
				create(filename, filesize);
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
				printf("Unknown command\n");
			}
		}
		fclose(disk);
	}

//	printf("\n");
	return 0;
}


int findfile(char *filename, struct BMFSEntry *fileentry, int *entrynumber)
{
	int tint;

	for (tint = 0; tint < 64; tint++)
	{
		memcpy(pentry, Directory+(tint*64), 64);
		if (entry.FileName[0] == 0x00)				// End of directory
		{
			tint = 64;
		}
		else if (entry.FileName[0] == 0x01)			// Emtpy entry
		{
			// Ignore
		}
		else										// Valid entry
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


void list()
{
	int tint;

	printf("%s Disk Size: %d MiB\n==============\n", diskname, disksize);
	for (tint = 0; tint < 64; tint++)			// Max 64 entries
	{
		memcpy(pentry, Directory+(tint*64), 64);
		if (entry.FileName[0] == 0x00)				// End of directory, bail out
		{
			tint = 64;
		}
		else if (entry.FileName[0] == 0x01)			// Emtpy entry
		{
			// Ignore
		}
		else										// Valid entry
		{
			printf("%s, Size:%lld B of %lld MiB\n", entry.FileName, entry.FileSize, (entry.ReservedBlocks*2));
//			printf("%s, Start:%lld, Resv:%lld MiB, Size:%lld B\n", entry.FileName, entry.StartingBlock, (entry.ReservedBlocks*2), entry.FileSize);
		}
	}
}


void create(char *filename, unsigned long long maxsize)
{
	struct BMFSEntry tempentry;
	int slot;
	
	if (maxsize % 2 != 0)
		maxsize++;

	if (findfile(filename, &tempentry, &slot) == 0)
	{
		printf("Creating new file..\n");
	}
	else
	{
		printf("File already exists.\n");
	}
}


void read(char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int tint, slot;

	if (0 == findfile(filename, &tempentry, &slot))
	{
		printf("File not found in BMFS.\n");
	}
	else
	{
		printf("Reading '%s' from BMFS to local file... ", filename);
		if ((tfile = fopen(tempentry.FileName, "wb")) == NULL)
		{
			printf("Error opening local file '%s'\n", tempentry.FileName);
		}
		else
		{
			fseek(disk, tempentry.StartingBlock*2097152, SEEK_SET); // Skip to the starting block in the disk
			for (tint=0; tint<tempentry.FileSize; tint++)
			{
				putc(getc(disk), tfile);			// This is really terrible.
				// TODO: Rework with fread and fwrite (ideally a 2MiB buffer)
			}
			fclose(tfile);
			printf("Complete\n");
		}
	}
}


void write(char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	char buffer[100];
	int tint, slot;
	unsigned long long tempfilesize;

	if (0 == findfile(filename, &tempentry, &slot))
	{
		printf("File not found in BMFS. A file entry must first be created.\n");
	}
	else
	{
		printf("Writing local file '%s' to BMFS... ", filename);
		if ((tfile = fopen(filename, "rb")) == NULL)
		{
			printf("Error opening local file '%s'\n", tempentry.FileName);
		}
		else
		{
			// Is there enough room in BMFS?
			fseek(tfile, 0, SEEK_END);
			tempfilesize = ftell(tfile);
			rewind(tfile);
			if ((tempentry.ReservedBlocks*2097152) < tempfilesize)
			{
				printf("Not enough reserved space in BMFS.\n");
			}
			else
			{
				fseek(disk, tempentry.StartingBlock*2097152, SEEK_SET); // Skip to the starting block in the disk
				for (tint=0; tint<tempfilesize; tint++)
				{
					putc(getc(tfile), disk);			// This is really terrible.
					// TODO: Rework with fread and fwrite (ideally a 2MiB buffer)
				}
				// Update directory
				memcpy(Directory+(slot*64)+48, &tempfilesize, 8);
				fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
				fwrite(Directory, 4096, 1, disk);			// Write new directory to disk				
				printf("Complete\n");
			}
			fclose(tfile);
		}
	}
}


void delete(char *filename)
{
	struct BMFSEntry tempentry;
	char delmarker = 0x01;
	int slot;

	if (0 == findfile(filename, &tempentry, &slot))
	{
		printf("File not found in BMFS.\n");
	}
	else
	{
		printf("Deleting file '%s' from BMFS... ", filename);
		// Update directory
		memcpy(Directory+(slot*64), &delmarker, 1);
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		fwrite(Directory, 4096, 1, disk);			// Write new directory to disk				
		printf("Complete\n");
	}
}


/* EOF */
