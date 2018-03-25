/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/fs.h>

#include <bmfs/disk.h>
#include <bmfs/entry.h>
#include <bmfs/file.h>
#include <bmfs/path.h>
#include <bmfs/table.h>
#include <bmfs/time.h>

#include <errno.h>

static int is_entry(struct BMFSEntry *entry,
                    const char *name,
                    uint64_t name_size) {

	if ((name_size == 0) || (name_size >= BMFS_FILE_NAME_MAX))
		return 0;

	for (uint64_t i = 0; i < name_size; i++) {
		if (name[i] != entry->Name[i])
			return 0;
	}

	return 1;
}

static int add_entry(struct BMFS *fs,
                     struct BMFSEntry *root,
                     const struct BMFSEntry *entry)
{
	if ((root->Size + BMFS_ENTRY_SIZE) > BMFS_BLOCK_SIZE)
		return -ENOSPC;

	uint64_t pos = root->Offset + root->Size;

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
                      uint64_t name_size)
{
	int err = bmfs_disk_seek(fs->Disk, parent_dir->Offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	uint64_t pos = 0;

	uint64_t pos_max = parent_dir->Size;

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

	return -ENOENT;
}

static int find_dir(struct BMFS *fs,
                    const struct BMFSEntry *parent_dir,
                    struct BMFSDir *dir,
                    const char *name,
                    uint64_t name_size)
{
	uint64_t pos = 0;

	uint64_t pos_max = parent_dir->Size;

	bmfs_dir_init(dir);

	bmfs_dir_set_disk(dir, fs->Disk);

	while (pos < pos_max) {

		int64_t offset = 0;
		offset += (int64_t) parent_dir->Offset;
		offset += (int64_t) pos;

		int err = bmfs_disk_seek(fs->Disk, offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		err = bmfs_dir_import(dir);
		if ((err != 0) && (err == -ENOTDIR) && (is_entry(&dir->Entry, name, name_size)))
			return err;
		else if ((err != 0) && (err != -ENOTDIR))
			return err;
		else if ((err == 0) && (is_entry(&dir->Entry, name, name_size)))
			return 0;

		pos += BMFS_ENTRY_SIZE;
	}

	return -ENOENT;
}

static int find_file(struct BMFS *fs,
                    const struct BMFSEntry *parent_dir,
                    struct BMFSFile *file,
                    const char *name,
                    uint64_t name_size)
{
	uint64_t pos = 0;

	uint64_t pos_max = parent_dir->Size;

	while (pos < pos_max) {

		int64_t offset = 0;
		offset += (int64_t) parent_dir->Offset;
		offset += (int64_t) pos;

		int err = bmfs_disk_seek(fs->Disk, offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		err = bmfs_file_import(file);
		if (err != 0)
		{
			if (err == -EISDIR)
				return err;
		}
		else if (is_entry(&file->Entry, name, name_size))
		{
			return 0;
		}

		pos += BMFS_ENTRY_SIZE;
	}

	return -ENOENT;
}

static int create_entry(struct BMFS *fs,
                        struct BMFSEntry *entry,
                        const char *path_string)
{
	/* Get the length of the path. */

	uint64_t path_size = 0;

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

		uint64_t name_size = parent.Length;
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

	uint64_t name_size = parent.Length;
	if ((name_size == 0) || (name_size >= BMFS_FILE_NAME_MAX))
		return -EINVAL;

	const char *name = parent.String;

	for (uint64_t i = 0; i < name_size; i++)
		entry->Name[i] = name[i];

	entry->Name[name_size] = 0;

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

	uint64_t path_size = 0;

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
		uint64_t name_size = parent.Length;
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

	uint64_t name_size = parent.Length;
	if (name_size >= BMFS_FILE_NAME_MAX)
		return -EINVAL;

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

	uint64_t path_size = 0;

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
		uint64_t name_size = parent.Length;
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

	uint64_t name_size = parent.Length;
	if (name_size == 0)
	{
		/* The root directory was passed. */
		return -EISDIR;
	}
	else if (name_size >= BMFS_FILE_NAME_MAX)
	{
		return -EINVAL;
	}

	const char *name = parent.String;

	err = find_file(fs, &root, file, name, name_size);
	if (err != 0)
		return err;

	return 0;
}

/* public functions */

void bmfs_init(struct BMFS *fs)
{
	bmfs_header_init(&fs->Header);
	fs->Disk = NULL;
}

void bmfs_done(struct BMFS *fs)
{
	if (fs->Disk != NULL) {
		bmfs_disk_done(fs->Disk);
		fs->Disk = NULL;
	}
}

void bmfs_set_disk(struct BMFS *fs,
                   struct BMFSDisk *disk) {
	if ((fs != NULL) && (disk != NULL)) {
		fs->Disk = disk;
	}
}

int bmfs_allocate(struct BMFS *fs, uint64_t size, uint64_t *offset_ptr)
{
	if ((fs == NULL) || (fs->Disk == NULL) || (offset_ptr == NULL))
		return -EFAULT;

	/* Check to see if the allocation
	 * table is already full. */

	if (fs->Header.TableEntryCount >= BMFS_TABLE_ENTRY_COUNT_MAX) {
		return -ENOSPC;
	}

	struct BMFSTableEntry entry;

	bmfs_table_entry_init(&entry);

	entry.Offset += sizeof(struct BMFSHeader);
	entry.Offset += sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_COUNT_MAX;
	entry.Offset += sizeof(struct BMFSEntry);
	entry.Used = size;
	/* Round to the nearest block size */
	entry.Reserved = ((size + (BMFS_BLOCK_SIZE - 1)) / BMFS_BLOCK_SIZE) * BMFS_BLOCK_SIZE;

	/* If there is existing allocations, then adjust the
	 * allocation to fit after the last region. */

	if (fs->Header.TableEntryCount > 0) {

		uint64_t entry_offset = 0;
		entry_offset += fs->Header.TableOffset;
		entry_offset += (fs->Header.TableEntryCount - 1) * sizeof(struct BMFSTableEntry);

		int err = bmfs_disk_seek(fs->Disk, entry_offset, BMFS_SEEK_SET);
		if (err != 0)
			return err;

		struct BMFSTableEntry last_entry;

		bmfs_table_entry_init(&last_entry);

		err = bmfs_table_entry_read(&last_entry, fs->Disk);
		if (err != 0)
			return err;

		entry.Offset = last_entry.Offset + last_entry.Reserved;
	}

	/* Check to make sure that the offset can fit into the disk. */

	if ((entry.Offset + entry.Reserved) > fs->Header.TotalSize) {
		return -ENOSPC;
	}

	/* Write the table entry. */

	uint64_t entry_offset = 0;
	entry_offset += fs->Header.TableOffset;
	entry_offset += sizeof(struct BMFSTableEntry) * fs->Header.TableEntryCount;

	int err = bmfs_disk_seek(fs->Disk, entry_offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_table_entry_write(&entry, fs->Disk);
	if (err != 0)
		return err;

	/* Update the header. */

	fs->Header.TableEntryCount++;

	err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_write(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	/* Assign the offset */

	*offset_ptr = entry.Offset;

	return 0;
}

int bmfs_allocate_mebibytes(struct BMFS *fs, uint64_t mebibytes, uint64_t *offset)
{
	return bmfs_allocate(fs, mebibytes * 1024 * 1024, offset);
}

int bmfs_check_signature(struct BMFS *fs)
{
	if (fs->Disk == NULL)
		return -EFAULT;

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
		return -EINVAL;

	return 0;
}

int bmfs_create_file(struct BMFS *fs, const char *path)
{
	if ((fs == NULL) || (fs->Disk == NULL) || (path == NULL))
		return -EFAULT;

	uint64_t offset = 0;

	int err = bmfs_allocate_mebibytes(fs, 2, &offset);
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
	if ((fs == NULL) || (fs->Disk == NULL) || (path == NULL))
		return -EFAULT;

	uint64_t offset = 0;

	int err = bmfs_allocate_mebibytes(fs, 2 /* 2MiB */, &offset);
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
	if ((fs == NULL) || (dir == NULL))
		return -EFAULT;

	int err = open_dir(fs, dir, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_open_file(struct BMFS *fs,
                   struct BMFSFile *file,
                   const char *path)
{
	if ((fs == NULL) || (file == NULL) || (path == NULL))
		return -EFAULT;

	return open_file(fs, file, path);
}

int bmfs_delete_file(struct BMFS *fs, const char *path)
{
	/* TODO
	 *  - open the entry
	 *  - delete the entry's allocation in the allocation table
	 *  - delete the entry within the parent directory
	 */

	(void) fs;
	(void) path;

	return -ENOSYS;
}

int bmfs_import(struct BMFS *fs)
{
	if ((fs == NULL) || (fs->Disk == NULL))
		return -EFAULT;

	int err = bmfs_disk_seek(fs->Disk, 0, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_read(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_format(struct BMFS *fs, uint64_t size)
{
	/* Write the file system header. */

	int err = bmfs_disk_seek(fs->Disk, 0UL, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	fs->Header.TotalSize = size;

	err = bmfs_header_write(&fs->Header, fs->Disk);
	if (err != 0)
		return err;

	/* Write the allocation table. */

	err = bmfs_disk_seek(fs->Disk, fs->Header.TableOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	for (uint64_t i = 0; i < BMFS_TABLE_ENTRY_COUNT_MAX; i++) {

		struct BMFSTableEntry table_entry;

		bmfs_table_entry_init(&table_entry);

		err = bmfs_table_entry_write(&table_entry, fs->Disk);
		if (err != 0)
			return err;
	}

	/* Write the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	bmfs_entry_set_type(&root, BMFS_TYPE_DIRECTORY);

	err = bmfs_allocate(fs, BMFS_BLOCK_SIZE, &root.Offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(fs->Disk, fs->Header.RootOffset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(&root, fs->Disk);
	if (err != 0)
		return err;

	return 0;
}
