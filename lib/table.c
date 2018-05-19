/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/table.h>

#include <bmfs/disk.h>
#include <bmfs/encoding.h>
#include <bmfs/entry.h>
#include <bmfs/errno.h>
#include <bmfs/host.h>
#include <bmfs/limits.h>

#include "crc32.h"

static int table_host_init(struct BMFSTable *table)
{
	if (table->Host == BMFS_NULL)
		return BMFS_EFAULT;

	if (table->HostData == BMFS_NULL)
	{
		table->HostData = bmfs_host_init(table->Host);
		if (table->HostData == BMFS_NULL)
			return BMFS_ENOMEM;
	}

	return 0;
}

static void *table_malloc(struct BMFSTable *table, bmfs_uint64 size)
{
	int err = table_host_init(table);
	if (err != 0)
		return BMFS_NULL;

	return bmfs_host_malloc(table->Host, table->HostData, size);
}

static void table_free(struct BMFSTable *table, void *addr)
{
	int err = table_host_init(table);
	if (err != 0)
		return;

	bmfs_host_free(table->Host, table->HostData, addr);
}

static bmfs_uint64 get_block_size(const struct BMFSTable *table)
{
	if (table->BlockSize == 0)
		return 1;
	else
		return table->BlockSize;
}

static bmfs_uint64 to_block_size(const struct BMFSTable *table,
                                 bmfs_uint64 size)
{
	bmfs_uint64 block_size = get_block_size(table);
	size = ((size + (block_size - 1)) / block_size) * block_size;
	return size;
}

static bmfs_uint32 bmfs_table_entry_checksum(const struct BMFSTableEntry *entry)
{
	char buf[24];

	bmfs_encode_uint64(entry->Offset, &buf[0x00]);
	bmfs_encode_uint64(entry->Reserved, &buf[0x08]);
	bmfs_encode_uint32(entry->Flags, &buf[0x10]);
	bmfs_encode_uint32(0x00, &buf[0x14]);

	return bmfs_crc32(0, buf, 24);
}

static int copy_over_data(struct BMFSTable *table,
                          bmfs_uint64 new_offset,
                          bmfs_uint64 old_offset,
                          bmfs_uint64 size)
{
	bmfs_uint64 block_size = 4096;

	if (block_size > size)
		block_size = size;

	void *block = table_malloc(table, block_size);
	if (block == BMFS_NULL)
		return BMFS_ENOMEM;

	bmfs_uint64 size_copied = 0;

	while (size_copied < size)
	{
		if ((size_copied + block_size) > size)
			block_size = size - size_copied;

		bmfs_uint64 read_count = 0;

		int err = bmfs_disk_seek(table->Disk, old_offset + size_copied, BMFS_SEEK_SET);
		if (err != 0)
		{
			table_free(table, block);
			return err;
		}

		err = bmfs_disk_read(table->Disk, block, block_size, &read_count);
		if (err != 0)
		{
			table_free(table, block);
			return err;
		}
		else if (read_count != block_size)
		{
			table_free(table, block);
			return BMFS_EIO;
		}

		err = bmfs_disk_seek(table->Disk, new_offset + size_copied, BMFS_SEEK_SET);
		if (err != 0)
		{
			table_free(table, block);
			return err;
		}

		bmfs_uint64 write_count = 0;

		err = bmfs_disk_write(table->Disk, block, block_size, &write_count);
		if (err != 0)
		{
			table_free(table, block);
			return err;
		}
		else if (write_count != block_size)
		{
			table_free(table, block);
			return BMFS_EIO;
		}

		size_copied += block_size;
	}

	table_free(table, block);

	return 0;
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
	unsigned char buf[24];

	bmfs_uint64 read_count = 0;

	int err = bmfs_disk_read(disk, buf, sizeof(buf), &read_count);
	if (err != 0)
		return err;
	else if (read_count != sizeof(buf))
		return BMFS_EIO;

	entry->Offset = bmfs_decode_uint64(&buf[0x00]);
	entry->Reserved = bmfs_decode_uint64(&buf[0x08]);
	entry->Flags = bmfs_decode_uint32(&buf[0x10]);
	entry->Checksum = bmfs_decode_uint32(&buf[0x14]);

	if (bmfs_table_entry_checksum(entry) != entry->Checksum)
		return BMFS_EINVAL;

	return 0;
}

int bmfs_table_entry_write(const struct BMFSTableEntry *entry,
                           struct BMFSDisk *disk)
{
	unsigned char buf[24];

	bmfs_encode_uint64(entry->Offset, &buf[0x00]);
	bmfs_encode_uint64(entry->Reserved, &buf[0x08]);
	bmfs_encode_uint32(entry->Flags, &buf[0x10]);
	bmfs_encode_uint32(bmfs_table_entry_checksum(entry), &buf[0x14]);

	bmfs_uint64 write_count = 0;

	int err = bmfs_disk_write(disk, buf, sizeof(buf), &write_count);
	if (err != 0)
		return err;
	else if (write_count != sizeof(buf))
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
	table->Host = BMFS_NULL;
	table->HostData = BMFS_NULL;
	table->Disk = BMFS_NULL;
	table->TableOffset = 0;
	table->EntryCount = 0;
	table->EntryIndex = 0;
	table->MaxOffset = 0;
	table->BlockSize = 128 * 1024;
	table->IgnoreDeleted = BMFS_TRUE;
	bmfs_table_entry_init(&table->CurrentEntry);
}

void bmfs_table_done(struct BMFSTable *table)
{
	if (table->Host != BMFS_NULL)
	{
		bmfs_host_done(table->Host, table->HostData);
		table->Host = BMFS_NULL;
		table->HostData = BMFS_NULL;
	}
}

void bmfs_table_set_host(struct BMFSTable *table,
                         const struct BMFSHost *host)
{
	if (table->Host != BMFS_NULL)
		bmfs_host_done(table->Host, table->HostData);

	table->Host = host;

	table->HostData = BMFS_NULL;
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
	entry.Offset = to_block_size(table, table->MinOffset);
	entry.Reserved = to_block_size(table, size);

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

int bmfs_table_realloc(struct BMFSTable *table,
                       bmfs_uint64 size,
                       bmfs_uint64 *offset)
{
	if (offset == BMFS_NULL)
		return BMFS_EFAULT;

	/* Find the old entry, so that the data
	 * can be copied over. */

	struct BMFSTableEntry *old_entry = bmfs_table_find(table, *offset);
	if (old_entry == BMFS_NULL)
		return BMFS_ENOENT;

	/* Check if it already fits, then we can
	 * bail out early. */

	if (old_entry->Reserved >= size)
		return 0;

	/* Save the offset and the size,
	 * so we can copy the data over */

	bmfs_uint64 old_offset = old_entry->Offset;
	bmfs_uint64 old_size = old_entry->Reserved;

	bmfs_table_end(table);

	bmfs_table_hide_deleted(table);

	struct BMFSTableEntry *last_entry = bmfs_table_previous(table);
	if (last_entry == BMFS_NULL)
		return BMFS_ENOSPC;

	bmfs_uint64 block_size = get_block_size(table);

	struct BMFSTableEntry new_entry;
	bmfs_table_entry_init(&new_entry);
	new_entry.Offset = last_entry->Offset + last_entry->Reserved;
	new_entry.Reserved = ((size + (block_size - 1)) / block_size) * block_size;

	int err = copy_over_data(table, new_entry.Offset, old_offset, old_size);
	if (err != 0)
		return err;

	err = bmfs_table_push(table, &new_entry);
	if (err != 0)
		return err;

	err = bmfs_table_free(table, old_offset);
	if (err != 0)
		return err;

	*offset = new_entry.Offset;

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
