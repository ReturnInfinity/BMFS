#include "stdlib.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>

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

static int bmfs_disk_file_read(void *file_ptr, void *buf, uint64_t len, uint64_t *read_len_ptr)
{
	if ((file_ptr == NULL)
	 || (buf == NULL))
		return -EFAULT;

	size_t read_len = fread(buf, 1, len, (FILE *)(file_ptr));
	if (read_len_ptr != NULL)
		*read_len_ptr = read_len;

	return 0;
}

static int bmfs_disk_file_write(void *file_ptr, const void *buf, uint64_t len, uint64_t *write_len_ptr)
{
	if ((file_ptr == NULL)
	 || (buf == NULL))
		return -EFAULT;

	size_t write_len = fwrite(buf, 1, len, (FILE *)(file_ptr));
	if (write_len_ptr != NULL)
		*write_len_ptr = write_len;

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
	disk->read = bmfs_disk_file_read;
	disk->write = bmfs_disk_file_write;

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
	FILE *disk = NULL;

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
		if (diskSize < BMFS_MINIMUM_DISK_SIZE)
		{
			printf( "Error: Disk size must be at least %llu bytes (%lluMiB)\n", BMFS_MINIMUM_DISK_SIZE, BMFS_MINIMUM_DISK_SIZE / (1024*1024));
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
		struct BMFSDisk tmp_disk;
		bmfs_disk_init_file(&tmp_disk, disk);
		bmfs_disk_format(&tmp_disk);
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

void bmfs_readfile(struct BMFSDisk *disk, const char *filename)
{
	struct BMFSEntry tempentry;
	FILE *tfile;
	int slot;
	unsigned long long bytestoread;
	char *buffer;

	if (bmfs_disk_find_file(disk, filename, &tempentry, &slot) != 0)
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
			bmfs_disk_seek(disk, tempentry.StartingBlock*BMFS_BLOCK_SIZE, SEEK_SET); // Skip to the starting block in the disk
			buffer = malloc(BMFS_BLOCK_SIZE);
			if (buffer == NULL)
			{
				printf("Error: Unable to allocate enough memory for buffer.\n");
			}
			else
			{
				while (bytestoread != 0)
				{
					if (bytestoread >= BMFS_BLOCK_SIZE)
					{
						if (bmfs_disk_read(disk, buffer, BMFS_BLOCK_SIZE, NULL) == 0)
						{
							fwrite(buffer, BMFS_BLOCK_SIZE, 1, tfile);
							bytestoread -= BMFS_BLOCK_SIZE;
						}
						else
						{
							printf("Error: Unexpected read length detected.\n");
							bytestoread = 0;
						}
					}
					else
					{
						if (bmfs_disk_read(disk, buffer, bytestoread, NULL) == 0)
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

void bmfs_writefile(struct BMFSDisk *disk, const char *filename)
{
	struct BMFSDir dir;
	struct BMFSEntry *entry;
	FILE *tfile;
	int retval;
	unsigned long long tempfilesize;
	char *buffer;

	if (bmfs_disk_read_dir(disk, &dir) != 0)
		return;

	entry = bmfs_dir_find(&dir, filename);
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
	if ((entry->ReservedBlocks*BMFS_BLOCK_SIZE) < tempfilesize)
	{
		fclose(tfile);
		printf("Error: Not enough reserved space in BMFS.\n");
		return;
	}

	retval = bmfs_disk_seek(disk, entry->StartingBlock*BMFS_BLOCK_SIZE, SEEK_SET);
	if (retval != 0)
		return;

	buffer = malloc(BMFS_BLOCK_SIZE);
	if (buffer == NULL)
	{
		fclose(tfile);
		printf("Error: Unable to allocate enough memory for buffer.\n");
	}

	while (tempfilesize != 0)
	{
		if (tempfilesize >= BMFS_BLOCK_SIZE)
		{
			retval = fread(buffer, BMFS_BLOCK_SIZE, 1, tfile);
			if (retval == 1)
			{
				bmfs_disk_write(disk, buffer, BMFS_BLOCK_SIZE, NULL);
				tempfilesize -= BMFS_BLOCK_SIZE;
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
				memset(buffer+(tempfilesize), 0, (BMFS_BLOCK_SIZE-tempfilesize)); // 0 the rest of the buffer
				bmfs_disk_write(disk, buffer, BMFS_BLOCK_SIZE, NULL);
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
	bmfs_disk_write_dir(disk, &dir);
	fclose(tfile);
}

