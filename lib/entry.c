/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/entry.h>
#include <bmfs/encoding.h>
#include <bmfs/disk.h>
#include <bmfs/errno.h>
#include <bmfs/limits.h>
#include <bmfs/time.h>

#include "crc32.h"
#include "memcpy.h"

#define BMFS_MASK_STATE 0xf0

#define BMFS_STATE_DELETED 0x10

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
	entry->Checksum = 0;
	entry->UserID = 0;
	entry->GroupID = 0;
	entry->EntryOffset = 0;
}

void bmfs_entry_copy(struct BMFSEntry *dst,
                     const struct BMFSEntry *src)
{
	*dst = *src;
}

bmfs_bool bmfs_entry_is_deleted(const struct BMFSEntry *entry)
{
	bmfs_uint64 state = entry->Flags & BMFS_MASK_STATE;
	if (state & BMFS_STATE_DELETED)
		return BMFS_TRUE;

	return BMFS_FALSE;
}

int bmfs_entry_read(struct BMFSEntry *entry,
                    struct BMFSDisk *disk)
{
	bmfs_uint64 disk_pos = 0;

	int err = bmfs_disk_tell(disk, &disk_pos);
	if (err != 0)
		return err;

	bmfs_uint64 read_size = 0;

	err = bmfs_disk_read(disk, entry, sizeof(struct BMFSEntry), &read_size);
	if (err != 0)
		return err;
	else if (read_size != BMFS_ENTRY_SIZE)
		return BMFS_EIO;

	entry->EntryOffset = (bmfs_uint64) disk_pos;

	return 0;
}

int bmfs_entry_save(struct BMFSEntry *entry,
                    struct BMFSDisk *disk)
{
	int err = bmfs_get_current_time(&entry->ModificationTime);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, entry->EntryOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(entry, disk);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_entry_write(struct BMFSEntry *entry,
                     struct BMFSDisk *disk) {

	unsigned char buf[BMFS_ENTRY_SIZE];

	bmfs_memcpy(buf, entry->Name, sizeof(entry->Name));

	bmfs_encode_uint64(entry->Offset,           &buf[BMFS_FILE_NAME_MAX]);
	bmfs_encode_uint64(entry->Size,             &buf[BMFS_FILE_NAME_MAX + 8]);
	bmfs_encode_uint64(entry->CreationTime,     &buf[BMFS_FILE_NAME_MAX + 16]);
	bmfs_encode_uint64(entry->ModificationTime, &buf[BMFS_FILE_NAME_MAX + 24]);
	bmfs_encode_uint32(entry->Flags,            &buf[BMFS_FILE_NAME_MAX + 32]);
	bmfs_encode_uint32(0 /* crc32 */,           &buf[BMFS_FILE_NAME_MAX + 36]);
	bmfs_encode_uint64(entry->UserID,           &buf[BMFS_FILE_NAME_MAX + 40]);
	bmfs_encode_uint64(entry->GroupID,          &buf[BMFS_FILE_NAME_MAX + 48]);
	bmfs_encode_uint64(0 /* unused */,          &buf[BMFS_FILE_NAME_MAX + 56]);

	entry->Checksum = bmfs_crc32(0, buf, BMFS_ENTRY_SIZE);

	bmfs_encode_uint32(entry->Checksum, &buf[BMFS_FILE_NAME_MAX + 36]);

	/* Get the current offset so we
	 * can set the entry offset field
	 * after the write operation. */

	bmfs_uint64 entry_offset = 0;

	int err = bmfs_disk_tell(disk, &entry_offset);
	if (err != 0)
		return err;

	bmfs_uint64 write_size = 0;

	err = bmfs_disk_write(disk, buf, sizeof(buf), &write_size);
	if (err != 0)
		return err;
	else if (write_size != BMFS_ENTRY_SIZE)
		return BMFS_EIO;

	entry->EntryOffset = entry_offset;

	return 0;
}

int bmfs_entry_get_offset(const struct BMFSEntry *entry, bmfs_uint64 *offset)
{
	if ((entry == BMFS_NULL) || (offset == BMFS_NULL))
		return BMFS_EFAULT;

	*offset = entry->Offset;

	return 0;
}

void bmfs_entry_set_deleted(struct BMFSEntry *entry)
{
	entry->Flags |= BMFS_STATE_DELETED;
}

void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename)
{
	bmfs_uint64 i;
	for (i = 0; (i < (BMFS_FILE_NAME_MAX - 1)) && (filename[i] != 0); i++)
		entry->Name[i] = filename[i];

	for (; i < BMFS_FILE_NAME_MAX; i++)
		entry->Name[i] = 0;
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

int bmfs_entry_is_file(const struct BMFSEntry *entry)
{
	if ((entry->Flags & BMFS_MASK_TYPE) == BMFS_FLAG_FILE)
		return 1;
	else
		return 0;
}
