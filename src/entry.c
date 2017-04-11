/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include "entry.h"
#include "limits.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void bmfs_entry_zero(struct BMFSEntry *entry)
{
	memset(entry, 0, sizeof(*entry));
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

