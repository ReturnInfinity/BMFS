/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include <bmfs/entry.h>
#include <bmfs/limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void bmfs_entry_init(struct BMFSEntry *entry)
{
	entry->FileName[0] = 1;
	entry->FileSize = 0;
	entry->StartingBlock = 0;
	entry->ReservedBlocks = 0;
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

	return bmfs_entry_cmp_filename(a, b->FileName);
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

	return bmfs_entry_cmp_starting_block(a, b->StartingBlock);
}

int bmfs_entry_cmp_filename(const struct BMFSEntry *entry, const char *filename)
{
	uint64_t i = 0;
	while (1)
	{
		if (entry->FileName[i] > filename[i])
			return 1;
		else if (entry->FileName[i] < filename[i])
			return -1;
		else if (entry->FileName[i] == 0)
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
	if (entry->StartingBlock > starting_block)
		return 1;
	else if (entry->StartingBlock < starting_block)
		return -1;
	return 0;
}

int bmfs_entry_get_offset(const struct BMFSEntry *entry, uint64_t *offset)
{
	if ((entry == NULL)
	 || (offset == NULL))
		return -EFAULT;

	*offset = entry->StartingBlock*BMFS_BLOCK_SIZE;

	return 0;
}

void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename)
{
	uint64_t i;
	for (i = 0; i < (BMFS_FILE_NAME_MAX - 1); i++)
	{
		if (filename[i] == 0)
			break;
		entry->FileName[i] = filename[i];
	}
	entry->FileName[i] = 0;
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

