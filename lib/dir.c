/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/dir.h>

#include <bmfs/disk.h>
#include <bmfs/errno.h>

static struct BMFSEntry *next_without_skipping(struct BMFSDir *dir)
{
	bmfs_uint64 offset = 0;
	offset += (bmfs_uint64) dir->CurrentIndex;
	offset *= BMFS_ENTRY_SIZE;
	offset += (bmfs_uint64) dir->Entry.Offset;

	if ((offset + BMFS_ENTRY_SIZE) > (dir->Entry.Offset + dir->Entry.Size))
		return BMFS_NULL;

	int err = bmfs_disk_seek(dir->Disk, offset, BMFS_SEEK_SET);
	if (err != 0)
		return BMFS_NULL;

	err = bmfs_entry_read(&dir->CurrentEntry, dir->Disk);
	if (err != 0)
		return BMFS_NULL;

	dir->CurrentIndex++;

	return &dir->CurrentEntry;
}

void bmfs_dir_init(struct BMFSDir *dir)
{
	dir->Disk = BMFS_NULL;
	dir->EntryOffset = 0;
	dir->CurrentIndex = 0;
	dir->IgnoreDeleted = BMFS_TRUE;
	bmfs_entry_init(&dir->Entry);
	bmfs_entry_init(&dir->CurrentEntry);
}

int bmfs_dir_save(struct BMFSDir *dir)
{
	int err = bmfs_entry_save(&dir->Entry, dir->Disk);
	if (err != 0)
		return err;

	return 0;
}

void bmfs_dir_view_deleted(struct BMFSDir *dir)
{
	dir->IgnoreDeleted = BMFS_FALSE;
}

void bmfs_dir_set_disk(struct BMFSDir *dir,
                       struct BMFSDisk *disk)
{
	if ((dir != BMFS_NULL) && (disk != BMFS_NULL))
	{
		dir->Disk = disk;
	}
}

int bmfs_dir_import(struct BMFSDir *dir)
{
	bmfs_uint64 entry_offset = 0;

	int err = bmfs_disk_tell(dir->Disk, &entry_offset);
	if (err != 0)
		return err;

	struct BMFSEntry entry;

	bmfs_entry_init(&entry);

	err = bmfs_entry_read(&entry, dir->Disk);
	if (err != 0)
		return err;
	else if (!bmfs_entry_is_directory(&entry))
		return BMFS_ENOTDIR;

	dir->Entry = entry;
	dir->EntryOffset = entry_offset;
	dir->CurrentIndex = 0;

	return 0;
}

struct BMFSEntry *bmfs_dir_next(struct BMFSDir *dir)
{
	struct BMFSEntry *entry = BMFS_NULL;

	for (;;)
	{
		entry = next_without_skipping(dir);
		if (entry == BMFS_NULL)
			return BMFS_NULL;

		if (!dir->IgnoreDeleted)
			return entry;

		if (!bmfs_entry_is_deleted(entry))
			return entry;
	}

	return entry;
}
