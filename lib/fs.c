/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/fs.h>

#include <bmfs/disk.h>
#include <bmfs/dir.h>
#include <bmfs/entry.h>
#include <bmfs/errno.h>
#include <bmfs/file.h>
#include <bmfs/host.h>
#include <bmfs/path.h>
#include <bmfs/status.h>
#include <bmfs/table.h>
#include <bmfs/time.h>
#include <bmfs/types.h>

#include <stdio.h>

static bmfs_uint64 get_block_size(const struct BMFS *bmfs)
{
	if (bmfs->Header.BlockSize == 0)
		return 1;
	else
		return bmfs->Header.BlockSize;
}

static int is_entry(struct BMFSEntry *entry,
                    const char *name,
                    bmfs_uint64 name_size) {

	if (bmfs_entry_is_deleted(entry))
		return 0;

	if ((name_size == 0) || (name_size >= BMFS_FILE_NAME_MAX))
		return 0;

	for (bmfs_uint64 i = 0; i < name_size; i++) {
		if (name[i] != entry->Name[i])
			return 0;
	}

	return 1;
}

static bmfs_bool can_fit_entry(struct BMFS *fs,
                               const struct BMFSEntry *root)
{
	struct BMFSTableEntry *entry = bmfs_table_find(&fs->Table, root->Offset);
	if (entry == BMFS_NULL)
		return BMFS_FALSE;

	if ((root->Size + BMFS_ENTRY_SIZE) > entry->Reserved)
		return BMFS_FALSE;

	return BMFS_TRUE;
}

static int add_entry(struct BMFS *fs,
                     struct BMFSEntry *root,
                     struct BMFSEntry *entry)
{
	if (!can_fit_entry(fs, root))
		return BMFS_ENOSPC;

	bmfs_uint64 pos = root->Offset + root->Size;

	int err = bmfs_disk_seek(fs->Disk, pos, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(entry, fs->Disk);
	if (err != 0)
		return err;

	/* Update directory size */

	err = bmfs_disk_seek(fs->Disk, root->EntryOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	root->Size += BMFS_ENTRY_SIZE;

	err = bmfs_entry_write(root, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}

static int find_entry(struct BMFS *fs,
                      const struct BMFSEntry *parent_dir,
                      struct BMFSEntry *entry,
                      const char *name,
                      bmfs_uint64 name_size)
{
	int err = bmfs_disk_seek(fs->Disk, parent_dir->Offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 pos = 0;

	bmfs_uint64 pos_max = parent_dir->Size;

	while (pos < pos_max) {

		bmfs_entry_init(entry);

		err = bmfs_entry_read(entry, fs->Disk);
		if (err != 0)
			return err;

		if (is_entry(entry, name, name_size)) {
			/* Found the entry */
			return 0;
		}

		pos += BMFS_ENTRY_SIZE;
	}

	return BMFS_ENOENT;
}

static int entry_exists(struct BMFS *fs,
                        const struct BMFSEntry *parent_dir,
                        const char *name,
                        bmfs_uint64 name_size)
{
	struct BMFSEntry tmp_entry;

	bmfs_entry_init(&tmp_entry);

	int err = find_entry(fs, parent_dir, &tmp_entry, name, name_size);
	if (err == BMFS_ENOENT)
		return 0;
	else
		return 1;
}

static int find_dir(struct BMFS *fs,
                    const struct BMFSEntry *parent_dir,
                    struct BMFSDir *dir,
                    const char *name,
                    bmfs_uint64 name_size)
{
	bmfs_uint64 pos = 0;

	bmfs_uint64 pos_max = parent_dir->Size;

	bmfs_dir_init(dir);

	bmfs_dir_set_disk(dir, fs->Disk);

	while (pos < pos_max) {

		bmfs_uint64 offset = 0;
		offset += parent_dir->Offset;
		offset += pos;

		int err = bmfs_disk_seek(fs->Disk, offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		err = bmfs_dir_import(dir);
		if ((err != 0) && (err == BMFS_ENOTDIR) && (is_entry(&dir->Entry, name, name_size)))
			return err;
		else if ((err != 0) && (err != BMFS_ENOTDIR))
			return err;
		else if ((err == 0) && (is_entry(&dir->Entry, name, name_size)))
			return 0;

		pos += BMFS_ENTRY_SIZE;
	}

	return BMFS_ENOENT;
}

static int find_file(struct BMFS *fs,
                    const struct BMFSEntry *parent_dir,
                    struct BMFSFile *file,
                    const char *name,
                    bmfs_uint64 name_size)
{
	bmfs_uint64 pos = 0;

	bmfs_uint64 pos_max = parent_dir->Size;

	while (pos < pos_max) {

		bmfs_uint64 offset = 0;
		offset += parent_dir->Offset;
		offset += pos;

		int err = bmfs_disk_seek(fs->Disk, offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		err = bmfs_file_import(file);
		if (err != 0)
		{
			if (err == BMFS_EISDIR)
				return err;
		}
		else if (is_entry(&file->Entry, name, name_size))
		{
			return 0;
		}

		pos += BMFS_ENTRY_SIZE;
	}

	return BMFS_ENOENT;
}

static int create_entry(struct BMFS *fs,
                        struct BMFSEntry *entry,
                        const char *path_string)
{
	/* Get the length of the path. */

	bmfs_uint64 path_size = 0;

	while (path_string[path_size] != 0)
		path_size++;

	/* Read the header to get the root directory
	 * offset. */

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, fs->Disk);
	if (err != 0)
		return err;

	/* Go to the root directory location. */

	err = bmfs_disk_seek(fs->Disk, header.RootOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	/* Read the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	err = bmfs_entry_read(&root, fs->Disk);
	if (err != 0)
		return err;

	/* Setup the path structures */

	struct BMFSPath path;

	bmfs_path_init(&path);

	bmfs_path_set(&path, path_string, path_size);

	struct BMFSPath parent;

	bmfs_path_init(&parent);

	/* Iterate the path until the
	 * basename is found */

	while ((bmfs_path_split_root(&path, &parent) == 0) && (path.Length > 0)) {

		bmfs_uint64 name_size = parent.Length;
		if (name_size == 0) {
			/* Reached the base name */
			break;
		}

		const char *name = parent.String;

		err = find_entry(fs, &root, &root, name, name_size);
		if (err != 0)
			return err;

		err = bmfs_disk_seek(fs->Disk, root.Offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;
	}

	/* Copy over file name */

	bmfs_uint64 name_size = parent.Length;
	if ((name_size == 0) || (name_size >= BMFS_FILE_NAME_MAX))
		return BMFS_EINVAL;

	const char *name = parent.String;

	for (bmfs_uint64 i = 0; i < name_size; i++)
		entry->Name[i] = name[i];

	entry->Name[name_size] = 0;

	/* Make sure that the entry doesn't exist. */

	if (entry_exists(fs, &root, name, name_size))
		return BMFS_EEXIST;

	err = add_entry(fs, &root, entry);
	if (err != 0)
		return err;

	return 0;
}

static int open_dir(struct BMFS *fs,
                    struct BMFSDir *dir,
                    const char *path_string)
{
	/* Get the length of the path. */

	bmfs_uint64 path_size = 0;

	while (path_string[path_size] != 0)
		path_size++;

	/* Read the header to get the root directory
	 * offset. */

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, fs->Disk);
	if (err != 0)
		return err;

	/* Go to the root directory location. */

	err = bmfs_disk_seek(fs->Disk, header.RootOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	/* Read the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	err = bmfs_entry_read(&root, fs->Disk);
	if (err != 0)
		return err;

	/* Setup the path structures */

	struct BMFSPath path;

	bmfs_path_init(&path);

	bmfs_path_set(&path, path_string, path_size);

	struct BMFSPath parent;

	bmfs_path_init(&parent);

	/* Iterate the path until the
	 * basename is found */

	while ((bmfs_path_split_root(&path, &parent) == 0) && (path.Length > 0))
	{
		bmfs_uint64 name_size = parent.Length;
		if (name_size == 0) {
			/* Reached the base name */
			break;
		}

		const char *name = parent.String;

		err = find_entry(fs, &root, &root, name, name_size);
		if (err != 0)
			return err;

		err = bmfs_disk_seek(fs->Disk, root.Offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;
	}

	/* Open the entry */

	bmfs_uint64 name_size = parent.Length;
	if (name_size >= BMFS_FILE_NAME_MAX)
		return BMFS_EINVAL;

	if (name_size == 0)
	{
		/* It's the root directory. */

		bmfs_dir_init(dir);

		bmfs_dir_set_disk(dir, fs->Disk);

		err = bmfs_disk_seek(fs->Disk, fs->Header.RootOffset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		return bmfs_dir_import(dir);
	}

	const char *name = parent.String;

	err = find_dir(fs, &root, dir, name, name_size);
	if (err != 0)
		return err;

	return 0;
}

static int open_file(struct BMFS *fs,
                     struct BMFSFile *file,
                     const char *path_string)
{
	/* Assign the disk to the file. */

	bmfs_file_set_disk(file, fs->Disk);

	/* Get the length of the path. */

	bmfs_uint64 path_size = 0;

	while (path_string[path_size] != 0)
		path_size++;

	/* Read the header to get the root directory
	 * offset. */

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, fs->Disk);
	if (err != 0)
		return err;

	/* Go to the root directory location. */

	err = bmfs_disk_seek(fs->Disk, header.RootOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	/* Read the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	err = bmfs_entry_read(&root, fs->Disk);
	if (err != 0)
		return err;

	/* Setup the path structures */

	struct BMFSPath path;

	bmfs_path_init(&path);

	bmfs_path_set(&path, path_string, path_size);

	struct BMFSPath parent;

	bmfs_path_init(&parent);

	/* Iterate the path until the
	 * basename is found */

	while ((bmfs_path_split_root(&path, &parent) == 0) && (path.Length > 0))
	{
		bmfs_uint64 name_size = parent.Length;
		if (name_size == 0) {
			/* Reached the base name */
			break;
		}

		const char *name = parent.String;

		err = find_entry(fs, &root, &root, name, name_size);
		if (err != 0)
			return err;

		err = bmfs_disk_seek(fs->Disk, root.Offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;
	}

	/* Open the entry */

	bmfs_uint64 name_size = parent.Length;
	if (name_size == 0)
	{
		/* The root directory was passed. */
		return BMFS_EISDIR;
	}
	else if (name_size >= BMFS_FILE_NAME_MAX)
	{
		return BMFS_EINVAL;
	}

	const char *name = parent.String;

	err = find_file(fs, &root, file, name, name_size);
	if (err != 0)
		return err;

	return 0;
}

static int delete_table_entry(struct BMFS *fs,
                              bmfs_uint64 offset)
{
	return bmfs_table_free(&fs->Table, offset);
}

static int delete_entry(struct BMFS *fs,
                        struct BMFSEntry *entry)
{
	int err = delete_table_entry(fs, entry->Offset);
	if (err != 0)
		return err;

	bmfs_entry_set_deleted(entry);

	err = bmfs_entry_save(entry, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}

/* public functions */

void bmfs_init(struct BMFS *fs)
{
	fs->Host = BMFS_NULL;
	fs->HostData = BMFS_NULL;
	fs->Disk = BMFS_NULL;
	fs->OpenFiles = BMFS_NULL;
	fs->OpenFileCount = 0;
	bmfs_header_init(&fs->Header);
	bmfs_table_init(&fs->Table);
}

void bmfs_done(struct BMFS *fs)
{
	for (bmfs_size i = 0; i < fs->OpenFileCount; i++)
	{
		bmfs_file_close(&fs->OpenFiles[i]);
	}

	if (fs->OpenFiles != BMFS_NULL)
	{
		bmfs_host_free(fs->Host, fs->HostData, fs->OpenFiles);
		fs->OpenFiles = BMFS_NULL;
	}

	if (fs->Disk != BMFS_NULL)
	{
		bmfs_disk_done(fs->Disk);
		fs->Disk = BMFS_NULL;
	}

	if (fs->Host != BMFS_NULL)
	{
		bmfs_host_done(fs->Host, fs->HostData);
		fs->Host = BMFS_NULL;
		fs->HostData = BMFS_NULL;
	}

	bmfs_table_done(&fs->Table);
}

void bmfs_set_host(struct BMFS *fs,
                   const struct BMFSHost *host)
{
	if (fs->Host != BMFS_NULL)
		bmfs_host_done(fs->Host, fs->HostData);

	fs->Host = host;

	/* allocate implementation data when needed. */
	fs->HostData = BMFS_NULL;

	bmfs_table_set_host(&fs->Table, host);
}

void bmfs_set_disk(struct BMFS *fs,
                   struct BMFSDisk *disk) {

	if ((fs != BMFS_NULL) && (disk != BMFS_NULL)) {
		fs->Disk = disk;
		bmfs_table_set_disk(&fs->Table, disk);
	}
}

void bmfs_get_status(struct BMFS *fs,
                     struct BMFSStatus *status)
{
	status->TotalSize = fs->Header.TotalSize;

	struct BMFSTable *table = &fs->Table;

	bmfs_table_begin(table);

	bmfs_table_hide_deleted(table);

	for (;;)
	{
		const struct BMFSTableEntry *entry = bmfs_table_next(table);
		if (entry == BMFS_NULL)
			break;

		status->Reserved += entry->Reserved;
	}
}

int bmfs_allocate(struct BMFS *fs, bmfs_uint64 size, bmfs_uint64 *offset_ptr)
{
	return bmfs_table_alloc(&fs->Table, size, offset_ptr);
}

int bmfs_check_signature(struct BMFS *fs)
{
	if (fs->Disk == BMFS_NULL)
		return BMFS_EFAULT;

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, fs->Disk);
	if (err != 0)
		return err;

	if ((header.Signature[0] != 'B')
	 || (header.Signature[1] != 'M')
	 || (header.Signature[2] != 'F')
	 || (header.Signature[3] != 'S')
	 || (header.Signature[4] != 0)
	 || (header.Signature[5] != 0)
	 || (header.Signature[6] != 0)
	 || (header.Signature[7] != 0))
		return BMFS_EINVAL;

	return 0;
}

int bmfs_create_file(struct BMFS *fs, const char *path)
{
	if ((fs == BMFS_NULL) || (fs->Disk == BMFS_NULL) || (path == BMFS_NULL))
		return BMFS_EFAULT;

	bmfs_uint64 offset = 0;

	int err = bmfs_allocate(fs, get_block_size(fs), &offset);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_type(&entry, BMFS_TYPE_FILE);
	entry.Offset = offset;
	bmfs_get_current_time(&entry.CreationTime);
	bmfs_get_current_time(&entry.ModificationTime);

	err = create_entry(fs, &entry, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_create_dir(struct BMFS *fs, const char *path)
{
	if ((fs == BMFS_NULL) || (fs->Disk == BMFS_NULL) || (path == BMFS_NULL))
		return BMFS_EFAULT;

	bmfs_uint64 offset = 0;

	int err = bmfs_allocate(fs, get_block_size(fs), &offset);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_type(&entry, BMFS_TYPE_DIRECTORY);
	entry.Offset = offset;
	bmfs_get_current_time(&entry.CreationTime);
	bmfs_get_current_time(&entry.ModificationTime);

	err = create_entry(fs, &entry, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_open_dir(struct BMFS *fs,
                  struct BMFSDir *dir,
                  const char *path)
{
	if ((fs == BMFS_NULL) || (dir == BMFS_NULL))
		return BMFS_EFAULT;

	int err = open_dir(fs, dir, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_open_file(struct BMFS *fs,
                   struct BMFSFile *file,
                   const char *path)
{
	if ((fs == BMFS_NULL) || (file == BMFS_NULL) || (path == BMFS_NULL))
		return BMFS_EFAULT;

	return open_file(fs, file, path);
}

int bmfs_delete_file(struct BMFS *fs, const char *path)
{
	/* TODO
	 *  - open the entry
	 *  - delete the entry's allocation in the allocation table
	 *  - delete the entry within the parent directory
	 */

	struct BMFSFile file;

	bmfs_file_init(&file);

	file.Table = &fs->Table;

	int err = bmfs_open_file(fs, &file, path);
	if (err != 0)
		return err;

	err = delete_entry(fs, &file.Entry);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_delete_dir(struct BMFS *fs, const char *path)
{
	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	int err = bmfs_open_dir(fs, &dir, path);
	if (err != 0)
		return err;

	if (bmfs_dir_next(&dir) != BMFS_NULL)
		return BMFS_ENOTEMPTY;

	err = delete_entry(fs, &dir.Entry);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_delete_dir_recursively(struct BMFS *fs, const char *path)
{
	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	int err = bmfs_open_dir(fs, &dir, path);
	if (err != 0)
		return err;

	/* TODO : This isn't fully recursive.
	 * It does not handle the case that there
	 * are more directories contained within
	 * this one. */

	for (;;)
	{
		struct BMFSEntry *entry = bmfs_dir_next(&dir);
		if (entry == BMFS_NULL)
			break;

		err = delete_entry(fs, &dir.Entry);
		if (err != 0)
			return err;
	}

	err = delete_entry(fs, &dir.Entry);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_rename(struct BMFS *fs,
                const char *old_path,
                const char *new_path)
{
	/* TODO : use an open_entry function instead
	 * of using error checking. It will be more
	 * efficient. */

	/* The old entry will contain the proper
	 * modification times, creation times, flags,
	 * data offset, and data size. We'll need it
	 * to copy the data over to the new entry. */

	struct BMFSEntry old_entry;

	bmfs_entry_init(&old_entry);

	/* Open the existing file so that we
	 * can get a hold of the entry structure. */

	struct BMFSFile old_file;

	old_file.Table = &fs->Table;

	bmfs_file_init(&old_file);

	int err = bmfs_open_file(fs, &old_file, old_path);
	if (err == BMFS_EISDIR)
	{
		struct BMFSDir dir;

		bmfs_dir_init(&dir);

		err = bmfs_open_dir(fs, &dir, old_path);
		if (err != 0)
			return err;

		bmfs_entry_copy(&old_entry, &dir.Entry);
	}
	else if (err != 0)
	{
		return err;
	}
	else
	{
		bmfs_entry_copy(&old_entry, &old_file.Entry);
	}

	/* Make a copy of the entry, before we mark
	 * it as deleted. */

	struct BMFSEntry new_entry;

	bmfs_entry_init(&new_entry);

	bmfs_entry_copy(&new_entry, &old_entry);

	/* Delete the entry without removing
	 * the entry in the allocation table. */

	bmfs_entry_set_deleted(&old_file.Entry);

	/* Ensure that the old directory knows
	 * that the entry is no longer active */

	err = bmfs_entry_save(&old_file.Entry, fs->Disk);
	if (err != 0)
		return err;

	/* Create the entry in the new location,
	 * containing all of its old information except
	 * for the entry offset and name, which are
	 * set in this function. */

	err = create_entry(fs, &new_entry, new_path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_import(struct BMFS *fs)
{
	if ((fs == BMFS_NULL) || (fs->Disk == BMFS_NULL))
		return BMFS_EFAULT;

	/* Read the file system header. */

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_read(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	err = bmfs_header_check(&fs->Header);
	if (err != 0)
		return err;

	/* Setup the allocation table */

	bmfs_table_set_offset(&fs->Table, fs->Header.TableOffset);

	bmfs_uint64 min_offset = 0;
	min_offset += sizeof(struct BMFSHeader);
	min_offset += sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_SIZE;
	min_offset += sizeof(struct BMFSEntry);
	bmfs_table_set_min_offset(&fs->Table, min_offset);

	bmfs_table_set_max_offset(&fs->Table, fs->Header.TotalSize);

	bmfs_table_set_count(&fs->Table, fs->Header.TableEntryCount);

	return 0;
}

int bmfs_export(struct BMFS *fs)
{
	if ((fs == BMFS_NULL) || (fs->Disk == BMFS_NULL))
		return BMFS_EFAULT;

	fs->Header.TableEntryCount = fs->Table.EntryCount;

	int err = bmfs_table_save_all(&fs->Table);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_write(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_format(struct BMFS *fs, bmfs_uint64 size)
{
	/* Setup the allocation table. */

	bmfs_table_set_offset(&fs->Table, fs->Header.TableOffset);

	bmfs_uint64 min_offset = 0;
	min_offset += sizeof(struct BMFSHeader);
	min_offset += sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_SIZE;
	min_offset += sizeof(struct BMFSEntry);
	bmfs_table_set_min_offset(&fs->Table, min_offset);

	bmfs_table_set_max_offset(&fs->Table, size);

	/* Create and write the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	bmfs_entry_set_type(&root, BMFS_TYPE_DIRECTORY);

	int err = bmfs_allocate(fs, BMFS_BLOCK_SIZE, &root.Offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(fs->Disk, fs->Header.RootOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(&root, fs->Disk);
	if (err != 0)
		return err;

	/* Write the allocation table. */

	err = bmfs_table_save_all(&fs->Table);
	if (err != 0)
		return err;

	/* Write the file system header. */

	fs->Header.TableEntryCount = fs->Table.EntryCount;

	fs->Header.TotalSize = size;

	err = bmfs_disk_seek(fs->Disk, 0UL, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_write(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}

void bmfs_set_block_size(struct BMFS *bmfs,
                         bmfs_uint64 block_size)
{
	if (block_size > 0)
	{
		bmfs_table_set_block_size(&bmfs->Table, block_size);

		bmfs->Header.BlockSize = block_size;
	}
}
