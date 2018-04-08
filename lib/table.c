/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/table.h>

#include <bmfs/disk.h>
#include <bmfs/errno.h>
#include <bmfs/entry.h>
#include <bmfs/limits.h>

#include <stdio.h>

static bmfs_uint64 get_block_size(const struct BMFSTable *table)
{
	if (table->BlockSize == 0)
		return 1;
	else
		return table->BlockSize;
}

static bmfs_uint32 bmfs_table_entry_checksum(const struct BMFSTableEntry *entry)
{
	bmfs_uint32 checksum = 0;
	checksum += (0x0f0f0f0f ^ entry->Offset);
	checksum += (0x0f00f00f ^ entry->Reserved);
	checksum ^= entry->Flags;

	return checksum;
}

#define BMFS_TABLE_FLAG_DELETED 0x01

void bmfs_table_entry_init(struct BMFSTableEntry *entry)
{
	entry->Offset = 0;
	entry->Reserved = 0;
	entry->Flags = 0;
	entry->Checksum = 0;
}

void bmfs_table_entry_copy(struct BMFSTableEntry *dst,
                           const struct BMFSTableEntry *src)
{
	dst->Offset = src->Offset;
	dst->Reserved = src->Reserved;
	dst->Flags = src->Flags;
	dst->Checksum = src->Checksum;
}

int bmfs_table_entry_check(const struct BMFSTableEntry *entry)
{
	if (bmfs_table_entry_checksum(entry) != entry->Checksum)
		return BMFS_EINVAL;

	return 0;
}

int bmfs_table_entry_read(struct BMFSTableEntry *entry,
                          struct BMFSDisk *disk)
{
	bmfs_uint64 read_count = 0;

	int err = bmfs_disk_read(disk, entry, sizeof(*entry), &read_count);
	if (err != 0)
		return err;
	else if (read_count != sizeof(*entry))
		return BMFS_EIO;

	return 0;
}

int bmfs_table_entry_write(const struct BMFSTableEntry *entry,
                           struct BMFSDisk *disk)
{
	bmfs_uint64 write_count = 0;

	int err = bmfs_disk_write(disk, entry, sizeof(*entry), &write_count);
	if (err != 0)
		return err;
	else if (write_count != sizeof(*entry))
		return BMFS_EIO;

	return 0;
}

void bmfs_table_entry_set_deleted(struct BMFSTableEntry *entry)
{
	entry->Flags |= BMFS_TABLE_FLAG_DELETED;
}

bmfs_bool bmfs_table_entry_is_deleted(const struct BMFSTableEntry *entry)
{
	if (entry->Flags & BMFS_TABLE_FLAG_DELETED)
		return BMFS_TRUE;

	return BMFS_FALSE;
}

void bmfs_table_init(struct BMFSTable *table)
{
	table->Disk = BMFS_NULL;
	table->TableOffset = 0;
	table->EntryCount = 0;
	table->EntryIndex = 0;
	table->MaxOffset = 0;
	table->BlockSize = 4096;
	table->IgnoreDeleted = BMFS_TRUE;
	bmfs_table_entry_init(&table->CurrentEntry);
}

void bmfs_table_view_deleted(struct BMFSTable *table)
{
	table->IgnoreDeleted = BMFS_FALSE;
}

void bmfs_table_hide_deleted(struct BMFSTable *table)
{
	table->IgnoreDeleted = BMFS_TRUE;
}

struct BMFSTableEntry *bmfs_table_find(struct BMFSTable *table,
                                       bmfs_uint64 offset)
{
	bmfs_table_begin(table);

	bmfs_table_hide_deleted(table);

	for (;;)
	{
		struct BMFSTableEntry *entry = bmfs_table_next(table);
		if (entry == BMFS_NULL)
			break;
		else if (entry->Offset == offset)
			return entry;
	}

	return BMFS_NULL;
}

int bmfs_table_alloc(struct BMFSTable *table,
                     bmfs_uint64 size,
                     bmfs_uint64 *offset)
{
	/* Check to see if the allocation
	 * table is already full. */

	if (table->EntryCount >= BMFS_TABLE_ENTRY_COUNT_MAX) {
		return BMFS_ENOSPC;
	}

	struct BMFSTableEntry entry;

	bmfs_table_entry_init(&entry);

	entry.Offset = table->MinOffset;

	bmfs_uint64 block_size = get_block_size(table);

	/* Round to the nearest block size */
	entry.Reserved = ((size + (block_size - 1)) / block_size) * block_size;

	/* If there is existing allocations, then adjust the
	 * allocation to fit after the last region. */

	bmfs_table_end(table);

	bmfs_table_hide_deleted(table);

	struct BMFSTableEntry *end = bmfs_table_previous(table);
	if (end != BMFS_NULL)
		entry.Offset = end->Offset + end->Reserved;

	/* Check to make sure that the offset can fit into the disk. */

	if ((entry.Offset + entry.Reserved) > table->MaxOffset) {
		return BMFS_ENOSPC;
	}

	/* Write the table entry. */

	int err = bmfs_table_push(table, &entry);
	if (err != 0)
		return err;

	/* Assign the offset */

	*offset = entry.Offset;

	return 0;
}

int bmfs_table_push(struct BMFSTable *table,
                    struct BMFSTableEntry *entry)
{
	bmfs_table_end(table);

	bmfs_table_entry_copy(&table->CurrentEntry, entry);

	int err = bmfs_table_save(table);
	if (err != 0)
		return err;

	table->EntryCount++;

	return 0;
}

int bmfs_table_free(struct BMFSTable *table,
                    bmfs_uint64 offset)
{
	bmfs_table_begin(table);

	for (;;)
	{
		struct BMFSTableEntry *entry = bmfs_table_next(table);
		if (entry == BMFS_NULL)
			return BMFS_ENOENT;
		else if (entry->Offset != offset)
			continue;

		bmfs_table_entry_set_deleted(entry);

		int err = bmfs_table_save(table);
		if (err != 0)
			return err;

		return 0;
	}

	return BMFS_ENOSYS;
}

void bmfs_table_begin(struct BMFSTable *table)
{
	table->EntryIndex = 0;
}

void bmfs_table_end(struct BMFSTable *table)
{
	table->EntryIndex = table->EntryCount;
}

static struct BMFSTableEntry *next_entry(struct BMFSTable *table)
{
	if (table->Disk == BMFS_NULL)
		return BMFS_NULL;
	else if (table->EntryIndex >= table->EntryCount)
		return BMFS_NULL;

	bmfs_uint64 entry_offset = 0;
	entry_offset += table->TableOffset;
	entry_offset += sizeof(struct BMFSTableEntry) * table->EntryIndex;

	int err = bmfs_disk_seek(table->Disk, entry_offset, BMFS_SEEK_SET);
	if (err != 0)
		return BMFS_NULL;

	err = bmfs_table_entry_read(&table->CurrentEntry, table->Disk);
	if (err != 0)
		return BMFS_NULL;

	table->EntryIndex++;

	return &table->CurrentEntry;
}

static struct BMFSTableEntry *prev_entry(struct BMFSTable *table)
{
	if (table->EntryIndex == 0)
		return BMFS_NULL;

	table->EntryIndex--;

	bmfs_uint64 entry_offset = 0;
	entry_offset += table->TableOffset;
	entry_offset += sizeof(struct BMFSTableEntry) * table->EntryIndex;

	int err = bmfs_disk_seek(table->Disk, entry_offset, BMFS_SEEK_SET);
	if (err != 0)
		return BMFS_NULL;

	err = bmfs_table_entry_read(&table->CurrentEntry, table->Disk);
	if (err != 0)
		return BMFS_NULL;

	return &table->CurrentEntry;
}

struct BMFSTableEntry *bmfs_table_next(struct BMFSTable *table)
{
	for (;;)
	{
		struct BMFSTableEntry *entry = next_entry(table);
		if (entry == BMFS_NULL)
			return BMFS_NULL;
		else if (!bmfs_table_entry_is_deleted(entry) || !table->IgnoreDeleted)
			return entry;
	}

	return BMFS_NULL;
}

struct BMFSTableEntry *bmfs_table_previous(struct BMFSTable *table)
{
	for (;;)
	{
		struct BMFSTableEntry *entry = prev_entry(table);
		if (entry == BMFS_NULL)
			return BMFS_NULL;
		else if (!bmfs_table_entry_is_deleted(entry) || !table->IgnoreDeleted)
			return entry;
	}

	return BMFS_NULL;
}

void bmfs_table_set_disk(struct BMFSTable *table,
                         struct BMFSDisk *disk)
{
	if (disk != BMFS_NULL)
		table->Disk = disk;
}

void bmfs_table_set_offset(struct BMFSTable *table,
                           bmfs_uint64 offset)
{
	table->TableOffset = offset;
}

void bmfs_table_set_count(struct BMFSTable *table,
                          bmfs_uint64 count)
{
	table->EntryCount = count;
}

void bmfs_table_set_block_size(struct BMFSTable *table,
                               bmfs_uint64 block_size)
{
	if (block_size > 0)
		table->BlockSize = block_size;
}

int bmfs_table_import(struct BMFSTable *table)
{
	if (table->Disk == BMFS_NULL)
		return BMFS_EFAULT;

	return 0;
}

int bmfs_table_save(struct BMFSTable *table)
{
	if (table->Disk == BMFS_NULL)
		return BMFS_EFAULT;

	bmfs_uint64 entry_offset = 0;
	entry_offset += table->TableOffset;
	entry_offset += sizeof(struct BMFSTableEntry) * table->EntryIndex;

	int err = bmfs_disk_seek(table->Disk, entry_offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_table_entry_write(&table->CurrentEntry, table->Disk);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_table_save_all(struct BMFSTable *table)
{
	if (table->Disk == BMFS_NULL)
		return BMFS_EFAULT;

	bmfs_table_begin(table);

	bmfs_table_view_deleted(table);

	for (;;)
	{
		struct BMFSTableEntry *entry = bmfs_table_next(table);
		if (entry == BMFS_NULL)
			break;

		entry->Checksum = bmfs_table_entry_checksum(entry);

		table->EntryIndex--;

		int err = bmfs_table_save(table);
		if (err != 0)
		{
			bmfs_table_hide_deleted(table);
			return err;
		}

		table->EntryIndex++;
	}

	bmfs_table_hide_deleted(table);

	return 0;
}

void bmfs_table_set_max_offset(struct BMFSTable *table,
                               bmfs_uint64 max_offset)
{
	table->MaxOffset = max_offset;
}

void bmfs_table_set_min_offset(struct BMFSTable *table,
                               bmfs_uint64 min_offset)
{
	table->MinOffset = min_offset;
}
