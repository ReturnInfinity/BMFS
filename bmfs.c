/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */

// Create a new 128MB disk image
// dd if=/dev/zero of=test.img bs=1m count=128

/* Global includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global defines */
struct BMFSEntry
{
	char FileName[32];
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
char fs_tag[] = "BMFS";
char s_list[] = "list";
char s_format[] = "format";
char s_create[] = "create";
char s_read[] = "read";
char s_write[] = "write";
char s_delete[] = "delete";
struct BMFSEntry entry;
void *pentry = &entry;
char *BlockMap;
char *FileBlocks;
char Directory[4096];
char DiskInfo[512];

/* Built-in functions */
int findfile(char *filename, struct BMFSEntry *fileentry, int *entrynumber);
void list();
void format();
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
		printf("BareMetal File System Utility v0.1 (2013 03 09)\n");
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
		printf("Error: Unable to open disk '%s'\n", diskname);
	}
	else	// Opened ok, is it a valid BMFS disk?
	{
		fseek(disk, 0, SEEK_END);
		disksize = ftell(disk) / 1048576;			// Disk size in MiB
		fseek(disk, 1024, SEEK_SET);				// Seek 1KiB in for disk information
		fread(DiskInfo, 512, 1, disk);				// Read 512 bytes to the DiskInfo buffer
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		fread(Directory, 4096, 1, disk);			// Read 4096 bytes to the Directory buffer
		rewind(disk);
		
		if (strcasecmp(DiskInfo, fs_tag) != 0)		// Is it a BMFS formatted disk?
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

	if (argc < 4)
	{
		if (strcasecmp(s_list, command) == 0)
		{
			list();
		}
		else if (strcasecmp(s_format, command) == 0)
		{
			format();
		}
		else
		{
			printf("Insufficient arguments.\n");
		}
	}
	else
	{
		if (strcasecmp(s_create, command) == 0)
		{
			printf("Maximum file size in MiB: ");
			fgets(tempstring, 32, stdin);			// Get up to 32 chars from the keyboard
			filesize = atoi(tempstring);
			if (filesize >= 1)
				create(filename, filesize);
			else
				printf("Error: Invalid file size.\n");
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

	printf("%s\nDisk Size: %d MiB\n", diskname, disksize);
	printf("Name                            |            Size (B)|      Reserved (MiB)\n");
	printf("==========================================================================\n");
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
			printf("%-32s %20lld %20lld\n", entry.FileName, entry.FileSize, (entry.ReservedBlocks*2));
		}
	}
}


void format()
{
	printf("!!! WARNING !!!\nThis will destroy the current file system on disk '%s'.\n", diskname);
	printf("Respond with 'YES' if so.\nFormat now?: ");
	fgets(tempstring, 32, stdin);			// Get up to 32 chars from the keyboard
	strtok(tempstring, "\n");				// Remove trailing newline
	if (strcmp(tempstring, "YES") == 0)
	{
		memset(DiskInfo, 0, 512);
		memset(Directory, 0, 4096);
		memcpy(DiskInfo, fs_tag, 4);				// Add the 'BMFS' tag
		fseek(disk, 1024, SEEK_SET);				// Seek 1KiB in for disk information
		fwrite(DiskInfo, 512, 1, disk);				// Write 512 bytes for the DiskInfo
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		fwrite(Directory, 4096, 1, disk);			// Write 4096 bytes for the Directory
		printf("Format complete.\n");
	}
	else
	{
		printf("Format aborted!\n");
	}
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

void create(char *filename, unsigned long long maxsize)
{
	struct BMFSEntry tempentry;
	int slot;
	
	if (maxsize % 2 != 0)
		maxsize++;

	if (findfile(filename, &tempentry, &slot) == 0)
	{
		unsigned long long blocks_requested = maxsize / 2; // how many blocks to allocate
		unsigned long long num_blocks = disksize / 2; // number of blocks in the disk
		char dir_copy[4096]; // copy of directory
		int num_used_entries = 0; // how many entries of Directory are either used or deleted
		int first_free_entry = -1; // where to put new entry
		int tint;
		struct BMFSEntry *pEntry;
		unsigned long long new_file_start = 0;
		unsigned long long prev_file_end = 1;

		printf("Creating new file...\n");

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
			printf("Cannot create file: no free directory entries.\n");
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

			unsigned long long this_file_start;
			pEntry = (struct BMFSEntry *)(dir_copy + tint * 64); // points to the current directory entry

			if (tint == num_used_entries || pEntry->FileName[0] == 0x01) 
				this_file_start = num_blocks - 1; // index of the last block
			else
				this_file_start = pEntry->StartingBlock;

			if (this_file_start - prev_file_end >= blocks_requested) 
			{ // fits here
				new_file_start = prev_file_end;
				break;
			}

			if (tint < num_used_entries)
				prev_file_end = pEntry->StartingBlock + pEntry->ReservedBlocks;
		}

		if (new_file_start == 0) 
		{
			printf("Cannot create file of size %lld MiB.\n", maxsize);
			return;
		}

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
		fseek(disk, 4096, SEEK_SET);				// Seek 4KiB in for directory
		fwrite(Directory, 4096, 1, disk);			// Write 4096 bytes for the Directory

		printf("Complete: file %s starts at block %lld, directory entry #%d.\n", filename, new_file_start, first_free_entry);
	}
	else
	{
		printf("Error: File already exists.\n");
	}
}


void read(char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int tint, slot;

	if (0 == findfile(filename, &tempentry, &slot))
	{
		printf("Error: File not found in BMFS.\n");
	}
	else
	{
		printf("Reading '%s' from BMFS to local file... ", filename);
		if ((tfile = fopen(tempentry.FileName, "wb")) == NULL)
		{
			printf("Error: Could not open local file '%s'\n", tempentry.FileName);
		}
		else
		{
			fseek(disk, tempentry.StartingBlock*2097152, SEEK_SET); // Skip to the starting block in the disk
			for (tint=0; tint<tempentry.FileSize; tint++)
			{
				putc(getc(disk), tfile);			// This is really terrible.
				// TODO: Rework with fread and fwrite (ideally with a 2MiB buffer)
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
	int tint, slot;
	unsigned long long tempfilesize;

	if (0 == findfile(filename, &tempentry, &slot))
	{
		printf("Error: File not found in BMFS. A file entry must first be created.\n");
	}
	else
	{
		printf("Writing local file '%s' to BMFS... ", filename);
		if ((tfile = fopen(filename, "rb")) == NULL)
		{
			printf("Error: Could not open local file '%s'\n", tempentry.FileName);
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
					// TODO: Rework with fread and fwrite (ideally with a 2MiB buffer)
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
		printf("Error: File not found in BMFS.\n");
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
