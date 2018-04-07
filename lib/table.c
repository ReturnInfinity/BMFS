/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/table.h>

#include <bmfs/disk.h>
#include <bmfs/errno.h>
#include <bmfs/limits.h>

#define BMFS_TABLE_FLAG_DELETED 0x01

void bmfs_table_entry_init(struct BMFSTableEntry *entry)
{
	entry->Offset = 0;
	entry->Used = 0;
	entry->Reserved = 0;
	entry->Flags = 0;
	entry->Checksum = 0;
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
	table->EntryIndex = 0;
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

static struct BMFSTableEntry *next_entry(struct BMFSTable *table)
{
	if (table->Disk == BMFS_NULL)
		return BMFS_NULL;
	else if (table->EntryIndex >= BMFS_TABLE_ENTRY_COUNT_MAX)
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

struct BMFSTableEntry *bmfs_table_next(struct BMFSTable *table)
{
	for (;;)
	{
		struct BMFSTableEntry *entry = next_entry(table);
		if (entry == BMFS_NULL)
			return BMFS_NULL;
		else if (bmfs_table_entry_is_deleted(entry) && !table->IgnoreDeleted)
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
