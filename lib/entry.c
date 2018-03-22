/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#include <bmfs/entry.h>
#include <bmfs/disk.h>
#include <bmfs/limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BMFS_MASK_TYPE 0x0f

#define BMFS_FLAG_FILE 0x01

#define BMFS_FLAG_DIR 0x02

#define BMFS_FLAG_FIFO 0x03

void bmfs_entry_init(struct BMFSEntry *entry)
{
	for (unsigned int i = 0; i < BMFS_FILE_NAME_MAX; i++)
		entry->Name[i] = 0;

	entry->Offset = 0;
	entry->Size = 0;
	entry->CreationTime = 0;
	entry->ModificationTime = 0;
	entry->Flags = 0;
	entry->Padding = 0;
}

int bmfs_entry_read(struct BMFSEntry *entry,
                    struct BMFSDisk *disk) {

	uint64_t read_size = 0;

	int err = bmfs_disk_read(disk, entry, sizeof(*entry), &read_size);
	if (err != 0)
		return err;
	else if (read_size != BMFS_ENTRY_SIZE)
		return -EIO;

	return 0;
}

int bmfs_entry_write(const struct BMFSEntry *entry,
                     struct BMFSDisk *disk) {

	uint64_t write_size = 0;

	int err = bmfs_disk_write(disk, entry, sizeof(*entry), &write_size);
	if (err != 0)
		return err;
	else if (write_size != BMFS_ENTRY_SIZE)
		return -EIO;

	return 0;
}

int bmfs_entry_cmp_by_filename(const struct BMFSEntry *a,
                               const struct BMFSEntry *b)
{
	if (bmfs_entry_is_empty(a)
	 && bmfs_entry_is_empty(b))
		return 0;

	if (bmfs_entry_is_empty(a))
		return 1;
	else if (bmfs_entry_is_empty(b))
		return -1;

	return bmfs_entry_cmp_filename(a, b->Name);
}

int bmfs_entry_cmp_by_starting_block(const struct BMFSEntry *a,
                                     const struct BMFSEntry *b)
{
	if (bmfs_entry_is_empty(a)
	 && bmfs_entry_is_empty(b))
		return 0;

	if (bmfs_entry_is_empty(a))
		return 1;
	else if (bmfs_entry_is_empty(b))
		return -1;

	return bmfs_entry_cmp_starting_block(a, b->Offset);
}

int bmfs_entry_cmp_filename(const struct BMFSEntry *entry, const char *filename)
{
	uint64_t i = 0;

	while (1)
	{
		if (entry->Name[i] > filename[i])
			return 1;
		else if (entry->Name[i] < filename[i])
			return -1;
		else if (entry->Name[i] == 0)
			break;
		else if (i >= BMFS_FILE_NAME_MAX)
			return -1;
		else
			i++;
	}

	return 0;
}

int bmfs_entry_cmp_starting_block(const struct BMFSEntry *entry, uint64_t starting_block)
{
	uint64_t starting_offset = starting_block * BMFS_BLOCK_SIZE;

	if (entry->Offset > starting_offset)
		return 1;
	else if (entry->Offset < starting_offset)
		return -1;

	return 0;
}

int bmfs_entry_get_offset(const struct BMFSEntry *entry, uint64_t *offset)
{
	if ((entry == NULL) || (offset == NULL))
		return -EFAULT;

	*offset = entry->Offset;

	return 0;
}

void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename)
{
	uint64_t i;
	for (i = 0; (i < (BMFS_FILE_NAME_MAX - 1)) && (filename[i] != 0); i++)
		entry->Name[i] = filename[i];

	for (; i < BMFS_FILE_NAME_MAX; i++)
		entry->Name[i] = 0;
}

void bmfs_entry_set_starting_block(struct BMFSEntry *entry, uint64_t starting_block)
{
	entry->Offset = starting_block * BMFS_BLOCK_SIZE;
}

void bmfs_entry_set_type(struct BMFSEntry *entry, enum BMFSEntryType type)
{
	entry->Flags &= ~BMFS_MASK_TYPE;

	switch (type)
	{
		case BMFS_TYPE_FILE:
			entry->Flags |= BMFS_FLAG_FILE;
			break;
		case BMFS_TYPE_DIRECTORY:
			entry->Flags |= BMFS_FLAG_DIR;
			break;
		default:
			break;
	}
}

int bmfs_entry_is_directory(const struct BMFSEntry *entry)
{
	if ((entry->Flags & BMFS_MASK_TYPE) == BMFS_FLAG_DIR)
		return 1;
	else
		return 0;
}

int bmfs_entry_is_empty(const struct BMFSEntry *entry)
{
	if ((entry->Flags & BMFS_MASK_TYPE) == 0)
		return 1;
	else
		return 0;
}

int bmfs_entry_is_file(const struct BMFSEntry *entry)
{
	if ((entry->Flags & BMFS_MASK_TYPE) == BMFS_FLAG_FILE)
		return 1;
	else
		return 0;
}

int bmfs_entry_is_terminator(const struct BMFSEntry *entry)
{
	/* No longer supported */
	(void) entry;
	return 0;
}
