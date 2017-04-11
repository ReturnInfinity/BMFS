/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

/* Global includes */
#include "libbmfs.h"
#include <errno.h>
#include <limits.h>

/* Global constants */
// Min disk size is 6MiB (three blocks of 2MiB each.)
const unsigned int minimumDiskSize = (6 * 1024 * 1024);
// Block size is 2MiB
const unsigned int blockSize = 2 * 1024 * 1024;


static int StartingBlockCmp(const void *pa, const void *pb);

/* file disk functions */

static int bmfs_disk_file_seek(void *file_ptr, int64_t offset, int whence)
{
	if (file_ptr == NULL)
		return -EFAULT;

	if (fseek((FILE *)(file_ptr), offset, whence) != 0)
		return -errno;

	return 0;
}

static int bmfs_disk_file_tell(void *file_ptr, int64_t *offset_ptr)
{
	if (file_ptr == NULL)
		return -EFAULT;

	int64_t offset = ftell((FILE *)(file_ptr));
	if (offset < 0)
		return -errno;

	if (offset_ptr != NULL)
		*offset_ptr = offset;

	return 0;
}

int bmfs_disk_init_file(struct BMFSDisk *disk, FILE *file)
{
	if ((disk == NULL)
	 || (file == NULL))
		return -EFAULT;

	disk->disk = file;
	disk->seek = bmfs_disk_file_seek;
	disk->tell = bmfs_disk_file_tell;

	return 0;
}

/* data disk functions */


void bmfs_entry_zero(struct BMFSEntry *entry)
{
	memset(entry, 0, sizeof(*entry));
}


void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename)
{
	snprintf(entry->FileName, sizeof(entry->FileName), "%s", filename);
}


void bmfs_entry_set_starting_block(struct BMFSEntry *entry, size_t starting_block)
{
	entry->StartingBlock = starting_block;
}


void bmfs_entry_set_reserved_blocks(struct BMFSEntry *entry, size_t reserved_blocks)
{
	entry->ReservedBlocks = reserved_blocks;
}


int bmfs_entry_is_empty(const struct BMFSEntry *entry)
{
	return entry->FileName[0] == 1;
}


int bmfs_entry_is_terminator(const struct BMFSEntry *entry)
{
	return entry->FileName[0] == 0;
}


int bmfs_disk_allocate_bytes(FILE *diskfile, size_t bytes, size_t *starting_block)
{
	if ((diskfile == NULL)
	 || (starting_block == NULL))
		return -EFAULT;

	if (bytes < blockSize)
		bytes = blockSize;

	struct BMFSDir dir;

	int err = bmfs_readdir(&dir, diskfile);
	if (err != 0)
		return err;

	err = bmfs_sortdir(&dir);
	if (err != 0)
		return err;

	size_t total_blocks;
	err = bmfs_disk_blocks(diskfile, &total_blocks);
	if (err != 0)
		return err;
	else if (total_blocks == 0)
		return -ENOSPC;

	size_t last_block = 1;
	size_t next_block = total_blocks;

	for (size_t i = 0; i < (64 - 1); i++)
	{
		struct BMFSEntry *entry = &dir.Entries[i];
		if (!(bmfs_entry_is_empty(entry))
		 && !(bmfs_entry_is_terminator(entry)))
			last_block = entry->StartingBlock + entry->ReservedBlocks;

		entry = &dir.Entries[i + 1];
		if ((bmfs_entry_is_empty(entry))
		 || (bmfs_entry_is_terminator(entry)))
			next_block = total_blocks;
		else
			next_block = entry->StartingBlock;

		size_t blocks_between = next_block - last_block;
		if ((blocks_between * blockSize) >= bytes)
		{
			/* found a spot between entries */
			*starting_block = last_block;
			return 0;
		}
	}

	return -ENOSPC;
}


int bmfs_disk_allocate_mebibytes(FILE *diskfile, size_t mebibytes, size_t *starting_block)
{
	return bmfs_disk_allocate_bytes(diskfile, mebibytes * 1024 * 1024, starting_block);
}


int bmfs_disk_bytes(FILE *diskfile, size_t *bytes)
{
	long disk_size;

	if (fseek(diskfile, 0, SEEK_END) != 0)
		return -errno;

	disk_size = ftell(diskfile);
	if (disk_size < 0)
		return -errno;

	if (bytes != NULL)
		*bytes = disk_size;

	return 0;
}


int bmfs_disk_mebibytes(FILE *diskfile, size_t *mebibytes)
{
	int err;

	err = bmfs_disk_bytes(diskfile, mebibytes);
	if (err != 0)
		return err;

	if (mebibytes != NULL)
		*mebibytes /= (1024 * 1024);

	return 0;
}


int bmfs_disk_blocks(FILE *diskfile, size_t *blocks)
{
	int err;

	err = bmfs_disk_bytes(diskfile, blocks);
	if (err != 0)
		return err;

	if (blocks != NULL)
		*blocks /= blockSize;

	return 0;
}


int bmfs_disk_create_file(FILE *diskfile, const char *filename, size_t mebibytes)
{
	int err;
	size_t starting_block;

	if (mebibytes % 2 != 0)
		mebibytes++;

	err = bmfs_disk_allocate_mebibytes(diskfile, mebibytes, &starting_block);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_zero(&entry);
	bmfs_entry_set_file_name(&entry, filename);
	bmfs_entry_set_starting_block(&entry, starting_block);
	bmfs_entry_set_reserved_blocks(&entry, mebibytes / 2);

	struct BMFSDir dir;

	err = bmfs_readdir(&dir, diskfile);
	if (err != 0)
		return err;

	err = bmfs_dir_add(&dir, &entry);
	if (err != 0)
		return err;

	err = bmfs_writedir(&dir, diskfile);
	if (err != 0)
		return err;

	return 0;
}


int bmfs_disk_delete_file(FILE *diskfile, const char *filename)
{
	struct BMFSDir dir;
	int err = bmfs_readdir(&dir, diskfile);
	if (err != 0)
		return err;

	struct BMFSEntry *entry;
	entry = bmfs_find(&dir, filename);
	if (entry == NULL)
		return -ENOENT;

	entry->FileName[0] = 1;

	return bmfs_writedir(&dir, diskfile);
}


int bmfs_disk_find_file(FILE *diskfile, const char *filename, struct BMFSEntry *fileentry, int *entrynumber)
{
	int err;
	struct BMFSDir dir;
	struct BMFSEntry *result;

	err = bmfs_readdir(&dir, diskfile);
	if (err != 0)
		return err;

	result = bmfs_find(&dir, filename);
	if (result == NULL)
		/* not found */
		return -ENOENT;

	if (fileentry)
		*fileentry = *result;

	if (entrynumber)
		*entrynumber = (result - &dir.Entries[0]) / sizeof(dir.Entries[0]);

	return 0;
}


int bmfs_disk_set_bytes(FILE *diskfile, size_t bytes)
{
	if (bytes < minimumDiskSize)
		bytes = minimumDiskSize;

	if (fseek(diskfile, bytes - 1, SEEK_SET) != 0)
		return -errno;

	if (fputc(0, diskfile) != 0)
		return -errno;

	return 0;
}


int bmfs_disk_set_mebibytes(FILE *diskfile, size_t mebibytes)
{
	return bmfs_disk_set_bytes(diskfile, mebibytes * 1024 * 1024);
}


int bmfs_disk_set_blocks(FILE *diskfile, size_t blocks)
{
	return bmfs_disk_set_blocks(diskfile, blocks * blockSize);
}


int bmfs_disk_check_tag(FILE *diskfile)
{
	char tag[4];

	if (fseek(diskfile, 1024, SEEK_SET) != 0)
		return -errno;

	if (fread(tag, 1, 4, diskfile) != 4)
		return -EINVAL;

	if ((tag[0] != 'B')
	 || (tag[1] != 'M')
	 || (tag[2] != 'F')
	 || (tag[3] != 'S'))
		return -EINVAL;

	return 0;
}


int bmfs_disk_write_tag(FILE *diskfile)
{
	if (fseek(diskfile, 1024, SEEK_SET) != 0)
		return -errno;

	if (fwrite("BMFS", 1, 4, diskfile) != 4)
		return -errno;

	return 0;
}


void bmfs_dir_zero(struct BMFSDir *dir)
{
	memset(dir, 0, sizeof(*dir));
}


int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry)
{
	for (size_t i = 0; i < 64; i++)
	{
		struct BMFSEntry *dst;
		dst = &dir->Entries[i];
		if (bmfs_entry_is_empty(dst))
		{
			*dst = *entry;
			return 0;
		}
		else if (bmfs_entry_is_terminator(dst))
		{
			*dst = *entry;
			if ((i + 1) < 64)
				/* make sure next entry
				 * indicates end of directory */
				dir->Entries[i + 1].FileName[0] = 0;
			return 0;
		}
	}
	/* ran out of entries */
	return -ENOSPC;
}


int bmfs_readdir(struct BMFSDir *dir, FILE *diskfile)
{
	bmfs_dir_zero(dir);

	if (fseek(diskfile, 4096, SEEK_SET) != 0)
		return -errno;

	if (fread(dir->Entries, 1, sizeof(dir->Entries), diskfile) != sizeof(dir->Entries))
		return -errno;

	return 0;
}


int bmfs_writedir(const struct BMFSDir *dir, FILE *diskfile)
{
	if (fseek(diskfile, 4096, SEEK_SET) != 0)
		return -errno;

	if (fwrite(dir->Entries, 1, sizeof(dir->Entries), diskfile) != sizeof(dir->Entries))
		return -errno;

	return 0;
}


int bmfs_sortdir(struct BMFSDir *dir)
{
	if (dir == NULL)
		return -EFAULT;
	size_t i;
	for (i = 0; i < 64; i++)
	{
		if (dir->Entries[i].FileName[0] == 0)
			break;
	}
	/* i is the number of used entries */
	qsort(dir->Entries, i, sizeof(dir->Entries[0]), StartingBlockCmp);
	return 0;
}


struct BMFSEntry * bmfs_find(struct BMFSDir *dir, const char *filename)
{
	int tint;
	for (tint = 0; tint < 64; tint++)
	{
		if (dir->Entries[tint].FileName[0] == 0)
			/* end of directory */
			break;
		else if (dir->Entries[tint].FileName[0] == 1)
			/* skip empty entry */
			continue;
		else if (strcmp(dir->Entries[tint].FileName, filename) != 0)
			/* not a match, skip this file */
			continue;

		/* file was found */
		return &dir->Entries[tint];
	}

	/* file not found */
	return NULL;
}


int bmfs_disk_format(FILE *diskfile)
{
	int err;

	err = bmfs_disk_set_bytes(diskfile, 0);
	if (err != 0)
		return err;

	err = bmfs_disk_write_tag(diskfile);
	if (err != 0)
		return err;

	struct BMFSDir dir;
	bmfs_dir_zero(&dir);
	if (fseek(diskfile, 4096, SEEK_SET) != 0)
		return -errno;
	fwrite(dir.Entries, 1, sizeof(dir.Entries), diskfile);

	return 0;
}


int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel)
{
	unsigned long long diskSize = 0;
	unsigned long long writeSize = 0;
	const char *bootFileType = NULL;
	size_t bufferSize = 50 * 1024;
	char * buffer = NULL;
	FILE *mbrFile = NULL;
	FILE *bootFile = NULL;
	FILE *kernelFile = NULL;
	int diskSizeFactor = 0;
	size_t chunkSize = 0;
	int ret = 0;
	size_t i;
	FILE *disk;

	// Determine how the second file will be described in output messages.
	// If a kernel file is specified too, then assume the second file is the
	// boot loader.  If no kernel file is specified, assume the boot loader
	// and kernel are combined into one system file.
	if (boot != NULL)
	{
		bootFileType = "boot loader";
		if (kernel == NULL)
		{
			bootFileType = "system";
		}
	}

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
				printf("Error: Disk size is too large\n");
				ret = 1;
			}
		}
		else if (i == 0) // No digits specified
		{
			printf("Error: A numeric disk size must be specified\n");
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
					case 'G':
						diskSizeFactor = 3;
						break;
					case 'T':
						diskSizeFactor = 4;
						break;
					case 'P':
						diskSizeFactor = 5;
						break;
					default:
						printf("Error: Invalid disk size string: '%s'\n", size);
						ret = 1;
						break;
			}

			// If this character is a valid unit indicator, but is not at the
			// end of the string, then the string is invalid.
			if (ret == 0 && size[i+1] != '\0')
			{
				printf("Error: Invalid disk size string: '%s'\n", size);
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
				printf("Error: Disk size is too large\n");
				ret = 1;
			}
		}
	}

	// Make sure the disk size is large enough.
	if (ret == 0)
	{
		if (diskSize < minimumDiskSize)
		{
			printf( "Error: Disk size must be at least %d bytes (%dMiB)\n", minimumDiskSize, minimumDiskSize / (1024*1024));
			ret = 1;
		}
	}

	// Open the Master boot Record file for reading.
	if (ret == 0 && mbr != NULL)
	{
		mbrFile = fopen(mbr, "rb");
		if (mbrFile == NULL )
		{
			printf("Error: Unable to open MBR file '%s'\n", mbr);
			ret = 1;
		}
	}

	// Open the boot loader file for reading.
	if (ret == 0 && boot != NULL)
	{
		bootFile = fopen(boot, "rb");
		if (bootFile == NULL )
		{
			printf("Error: Unable to open %s file '%s'\n", bootFileType, boot);
			ret = 1;
		}
	}

	// Open the kernel file for reading.
	if (ret == 0 && kernel != NULL)
	{
		kernelFile = fopen(kernel, "rb");
		if (kernelFile == NULL )
		{
			printf("Error: Unable to open kernel file '%s'\n", kernel);
			ret = 1;
		}
	}

	// Allocate buffer to use for filling the disk image with zeros.
	if (ret == 0)
	{
		buffer = (char *) malloc(bufferSize);
		if (buffer == NULL)
		{
			printf("Error: Failed to allocate buffer\n");
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
			printf("Error: Unable to open disk '%s'\n", diskname);
			ret = 1;
		}
	}

	// Fill the disk image with zeros.
	if (ret == 0)
	{
		double percent;
		memset(buffer, 0, bufferSize);
		writeSize = 0;
		while (writeSize < diskSize)
		{
			percent = writeSize;
			percent /= diskSize;
			percent *= 100;
			printf("Formatting disk: %llu of %llu bytes (%.0f%%)...\r", writeSize, diskSize, percent);
			chunkSize = bufferSize;
			if (chunkSize > diskSize - writeSize)
			{
				chunkSize = diskSize - writeSize;
			}
			if (fwrite(buffer, chunkSize, 1, disk) != 1)
			{
				printf("Error: Failed to write disk '%s'\n", diskname);
				ret = 1;
				break;
			}
			writeSize += chunkSize;
		}
		if (ret == 0)
		{
			printf("Formatting disk: %llu of %llu bytes (100%%)%9s\n", writeSize, diskSize, "");
		}
	}

	// Format the disk.
	if (ret == 0)
	{
		rewind(disk);
		bmfs_disk_format(disk);
	}

	// Write the master boot record if it was specified by the caller.
	if (ret == 0 && mbrFile != NULL)
	{
		fseek(disk, 0, SEEK_SET);
		if (fread(buffer, 512, 1, mbrFile) == 1)
		{
			if (fwrite(buffer, 512, 1, disk) != 1)
			{
				printf("Error: Failed to write disk '%s'\n", diskname);
				ret = 1;
			}
		}
		else
		{
			printf("Error: Failed to read file '%s'\n", mbr);
			ret = 1;
		}
	}

	// Write the boot loader if it was specified by the caller.
	if (ret == 0 && bootFile != NULL)
	{
		fseek(disk, 8192, SEEK_SET);
		for (;;)
		{
			chunkSize = fread( buffer, 1, bufferSize, bootFile);
			if (chunkSize > 0)
			{
				if (fwrite(buffer, chunkSize, 1, disk) != 1)
				{
					printf("Error: Failed to write disk '%s'\n", diskname);
					ret = 1;
				}
			}
			else
			{
				if (ferror(disk))
				{
					printf("Error: Failed to read file '%s'\n", boot);
					ret = 1;
				}
				break;
			}
		}
	}

	// Write the kernel if it was specified by the caller. The kernel must
	// immediately follow the boot loader on disk (i.e. no seek needed.)
	if (ret == 0 && kernelFile != NULL)
	{
		for (;;)
		{
			chunkSize = fread( buffer, 1, bufferSize, kernelFile);
			if (chunkSize > 0)
			{
				if (fwrite(buffer, chunkSize, 1, disk) != 1)
				{
					printf("Error: Failed to write disk '%s'\n", diskname);
					ret = 1;
				}
			}
			else
			{
				if (ferror(disk))
				{
					printf("Error: Failed to read file '%s'\n", kernel);
					ret = 1;
				}
				break;
			}
		}
	}

	// Close any files that were opened.
	if (mbrFile != NULL)
	{
		fclose(mbrFile);
	}
	if (bootFile != NULL)
	{
		fclose(bootFile);
	}
	if (kernelFile != NULL)
	{
		fclose(kernelFile);
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
		printf("Disk initialization complete.\n");
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

// Read a file from a BMFS volume
void bmfs_readfile(FILE *diskfile, char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int slot, retval;
	unsigned long long bytestoread;
	char *buffer;

	if (0 == bmfs_disk_find_file(diskfile, filename, &tempentry, &slot))
	{
		printf("Error: File not found in BMFS.\n");
	}
	else
	{
		if ((tfile = fopen(tempentry.FileName, "wb")) == NULL)
		{
			printf("Error: Could not open local file '%s'\n", tempentry.FileName);
		}
		else
		{
			bytestoread = tempentry.FileSize;
			fseek(diskfile, tempentry.StartingBlock*blockSize, SEEK_SET); // Skip to the starting block in the disk
			buffer = malloc(blockSize);
			if (buffer == NULL)
			{
				printf("Error: Unable to allocate enough memory for buffer.\n");
			}
			else
			{
				while (bytestoread != 0)
				{
					if (bytestoread >= blockSize)
					{
						retval = fread(buffer, blockSize, 1, diskfile);
						if (retval == 1)
						{
							fwrite(buffer, blockSize, 1, tfile);
							bytestoread -= blockSize;
						}
						else
						{
							printf("Error: Unexpected read length detected.\n");
							bytestoread = 0;
						}
					}
					else
					{
						retval = fread(buffer, bytestoread, 1, diskfile);
						if (retval == 1)
						{
							fwrite(buffer, bytestoread, 1, tfile);
							bytestoread = 0;
						}
						else
						{
							printf("Error: Unexpected read length detected.\n");
							bytestoread = 0;
						}
					}
				}
			}
			fclose(tfile);
		}
	}
}

unsigned long long bmfs_read(FILE *diskfile,
                             const char *filename,
                             void *buf,
                             unsigned long long len,
                             unsigned long long off)
{
	struct BMFSEntry tempentry;

	if (bmfs_disk_find_file(diskfile, filename, &tempentry, NULL) == 0)
	{
		memcpy(buf, "h", 1);
		return 1;
	}

	/* Make sure the offset doesn't overflow */
	if (off > tempentry.FileSize)
		off = tempentry.FileSize;

	/* Make sure the read length doesn't overflow */
	if ((off + len) > tempentry.FileSize)
		len = tempentry.FileSize - off;

	/* Skip to the starting block in the disk */
	fseek(diskfile, (tempentry.StartingBlock*blockSize) + off, SEEK_SET);

	return fread(buf, 1, len, diskfile);
}

int bmfs_write(FILE *diskfile,
               const char *filename,
               const void *buf,
               size_t len,
               off_t off)
{
	struct BMFSDir dir;
	if (bmfs_readdir(&dir, diskfile) != 0)
		return -ENOENT;

	struct BMFSEntry *entry;
	entry = bmfs_find(&dir, filename);
	if (entry == NULL)
		return -ENOENT;

	/* make sure fuse isn't
	 * screwing anything up */
	if (off < 0)
		off = 0;

	/* Make sure the offset doesn't overflow */
	if (off > (unsigned)(entry->ReservedBlocks*blockSize))
		off = entry->ReservedBlocks*blockSize;

	/* Make sure the read length doesn't overflow */
	if ((off+len) > (entry->ReservedBlocks*blockSize))
		len = (entry->ReservedBlocks*blockSize) - off;

	/* must be able to distinguish between bytes
	 * written and a negative error code */
	if (len > INT_MAX)
		len = INT_MAX;

	/* Skip to the starting block in the disk */
	fseek(diskfile, (entry->StartingBlock*blockSize) + off, SEEK_SET);

	size_t write_count = fwrite(buf, 1, len, diskfile);

	entry->FileSize += write_count;

	bmfs_writedir(&dir, diskfile);

	return write_count;
}

// Write a file to a BMFS volume
void bmfs_writefile(FILE *diskfile, char *filename)
{
	struct BMFSDir dir;
	struct BMFSEntry *entry;
	FILE *tfile;
	int retval;
	unsigned long long tempfilesize;
	char *buffer;

	if (bmfs_readdir(&dir, diskfile) != 0)
		return;

	entry = bmfs_find(&dir, filename);
	if (entry == NULL)
	{
		printf("Error: File not found in BMFS\n");
		printf("  A file must first be created\n");
		return;
	}

	if ((tfile = fopen(filename, "rb")) == NULL)
	{
		printf("Error: Could not open local file '%s'\n", entry->FileName);
		return;
	}

	// Is there enough room in BMFS?
	fseek(tfile, 0, SEEK_END);
	tempfilesize = ftell(tfile);
	rewind(tfile);
	if ((entry->ReservedBlocks*blockSize) < tempfilesize)
	{
		fclose(tfile);
		printf("Error: Not enough reserved space in BMFS.\n");
		return;
	}
	fseek(diskfile, entry->StartingBlock*blockSize, SEEK_SET); // Skip to the starting block in the disk

	buffer = malloc(blockSize);
	if (buffer == NULL)
	{
		fclose(tfile);
		printf("Error: Unable to allocate enough memory for buffer.\n");
	}

	while (tempfilesize != 0)
	{
		if (tempfilesize >= blockSize)
		{
			retval = fread(buffer, blockSize, 1, tfile);
			if (retval == 1)
			{
				fwrite(buffer, blockSize, 1, diskfile);
				tempfilesize -= blockSize;
			}
			else
			{
				printf("Error: Unexpected read length detected.\n");
				tempfilesize = 0;
			}
		}
		else
		{
			retval = fread(buffer, tempfilesize, 1, tfile);
			if (retval == 1)
			{
				memset(buffer+(tempfilesize), 0, (blockSize-tempfilesize)); // 0 the rest of the buffer
				fwrite(buffer, blockSize, 1, diskfile);
				tempfilesize = 0;
			}
			else
			{
				printf("Error: Unexpected read length detected.\n");
				tempfilesize = 0;
			}
		}
	}

	// Update directory
	tempfilesize = ftell(tfile);
	entry->FileSize = tempfilesize;
	bmfs_writedir(&dir, diskfile);
	fclose(tfile);
}


/* EOF */
