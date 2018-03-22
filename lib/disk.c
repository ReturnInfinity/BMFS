/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#include <bmfs/disk.h>
#include <bmfs/header.h>
#include <bmfs/limits.h>
#include <bmfs/path.h>
#include <bmfs/table.h>
#include <bmfs/time.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef _MSC_VER
#include <strings.h>
#endif /* _MSC_VER */

#include <stdio.h>

/* disk wrapper functions */

int bmfs_disk_seek(struct BMFSDisk *disk, int64_t offset, int whence)
{
	if ((disk == NULL)
	 || (disk->seek == NULL))
		return -EFAULT;

	return disk->seek(disk->disk, offset, whence);
}

int bmfs_disk_tell(struct BMFSDisk *disk, int64_t *offset)
{
	if ((disk == NULL)
	 || (disk->tell == NULL))
		return -EFAULT;

	return disk->tell(disk->disk, offset);
}

int bmfs_disk_read(struct BMFSDisk *disk, void *buf, uint64_t len, uint64_t *read_len)
{
	if ((disk == NULL)
	 || (disk->read == NULL))
		return -EFAULT;

	return disk->read(disk->disk, buf, len, read_len);
}

int bmfs_disk_write(struct BMFSDisk *disk, const void *buf, uint64_t len, uint64_t *write_len)
{
	if ((disk == NULL)
	 || (disk->write == NULL))
		return -EFAULT;

	return disk->write(disk->disk, buf, len, write_len);
}

/* private functions */

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

static int add_entry(struct BMFSDisk *disk,
                     const struct BMFSEntry *root,
                     const struct BMFSEntry *entry)
{
	uint64_t pos = root->Offset;

	int err = bmfs_disk_seek(disk, pos, SEEK_SET);
	if (err != 0)
		return err;

	uint64_t pos_max = pos + BMFS_BLOCK_SIZE;

	struct BMFSEntry tmp_entry;

	while (pos < pos_max) {

		bmfs_entry_init(&tmp_entry);

		err = bmfs_entry_read(&tmp_entry, disk);
		if (err != 0)
			return err;

		if (bmfs_entry_is_empty(&tmp_entry))
			break;

		pos += BMFS_ENTRY_SIZE;
	}

	if (pos >= pos_max)
		return -ENOSPC;

	err = bmfs_disk_seek(disk, pos, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(entry, disk);
	if (err != 0)
		return err;

	return 0;
}

static int find_entry(struct BMFSDisk *disk,
                      const struct BMFSEntry *parent_dir,
                      struct BMFSEntry *entry,
                      const char *name,
                      uint64_t name_size)
{
	int err = bmfs_disk_seek(disk, parent_dir->Offset, SEEK_SET);
	if (err != 0)
		return err;

	uint64_t pos = 0;

	uint64_t pos_max = BMFS_BLOCK_SIZE;

	while (pos < pos_max) {

		bmfs_entry_init(entry);

		err = bmfs_entry_read(entry, disk);
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

static int create_entry(struct BMFSDisk *disk,
                        struct BMFSEntry *entry,
                        const char *path_string)
{
	/* Get the length of the path. */

	uint64_t path_size = 0;

	while (path_string[path_size] != 0)
		path_size++;

	/* Read the header to get the root directory
	 * offset. */

	int err = bmfs_disk_seek(disk, 0, SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, disk);
	if (err != 0)
		return err;

	/* Go to the root directory location. */

	err = bmfs_disk_seek(disk, header.RootOffset, SEEK_SET);
	if (err != 0)
		return err;

	/* Read the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	err = bmfs_entry_read(&root, disk);
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

		err = find_entry(disk, &root, &root, name, name_size);
		if (err != 0)
			return err;

		err = bmfs_disk_seek(disk, root.Offset, SEEK_SET);
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

	err = add_entry(disk, &root, entry);
	if (err != 0)
		return err;

	return 0;
}

/* public functions */

void bmfs_disk_init(struct BMFSDisk *disk)
{
	disk->disk = NULL;
	disk->done = NULL;
	disk->seek = NULL;
	disk->tell = NULL;
	disk->read = NULL;
	disk->write = NULL;
}

void bmfs_disk_done(struct BMFSDisk *disk)
{
	if (disk->done != NULL)
		disk->done(disk->disk);
}

int bmfs_disk_read_root_dir(struct BMFSDisk *disk, struct BMFSDir *dir)
{
	int err = bmfs_disk_seek(disk, 4096, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_read(disk, dir->Entries, sizeof(dir->Entries), NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_write_root_dir(struct BMFSDisk *disk, const struct BMFSDir *dir)
{
	int err = bmfs_disk_seek(disk, 4096, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(disk, dir->Entries, sizeof(dir->Entries), NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_allocate(struct BMFSDisk *disk, uint64_t size, uint64_t *offset_ptr)
{
	if ((disk == NULL) || (offset_ptr == NULL))
		return -EFAULT;

	/* Read header to get table entry count
	 * and table offset. */

	int err = bmfs_disk_seek(disk, 0, SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, disk);
	if (err != 0)
		return err;

	/* Check to see if the allocation
	 * table is already full. */

	if (header.TableEntryCount >= BMFS_TABLE_ENTRY_COUNT_MAX) {
		return -ENOSPC;
	}

	struct BMFSTableEntry entry;

	bmfs_table_entry_init(&entry);

	entry.Offset += sizeof(struct BMFSHeader);
	entry.Offset += sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_COUNT_MAX;
	entry.Used = size;
	/* Round to the nearest block size */
	entry.Reserved = ((size + (BMFS_BLOCK_SIZE - 1)) / BMFS_BLOCK_SIZE) * BMFS_BLOCK_SIZE;

	/* If there is existing allocations, then adjust the
	 * allocation to fit after the last region. */

	if (header.TableEntryCount > 0) {

		uint64_t entry_offset = 0;
		entry_offset += header.TableOffset;
		entry_offset += (header.TableEntryCount - 1) * sizeof(struct BMFSTableEntry);

		int err = bmfs_disk_seek(disk, entry_offset, SEEK_SET);
		if (err != 0)
			return err;

		struct BMFSTableEntry last_entry;

		bmfs_table_entry_init(&last_entry);

		err = bmfs_table_entry_read(&last_entry, disk);
		if (err != 0)
			return err;

		entry.Offset = last_entry.Offset + last_entry.Reserved;
	}

	/* Check to make sure that the offset can fit into the disk. */

	if ((entry.Offset + entry.Reserved) > header.TotalSize) {
		return -ENOSPC;
	}

	/* Write the table entry. */

	uint64_t entry_offset = 0;
	entry_offset += header.TableOffset;
	entry_offset += sizeof(struct BMFSTableEntry) * header.TableEntryCount;

	err = bmfs_disk_seek(disk, entry_offset, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_table_entry_write(&entry, disk);
	if (err != 0)
		return err;

	/* Update the header. */

	header.TableEntryCount++;

	err = bmfs_disk_seek(disk, 0, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_header_write(&header, disk);
	if (err != 0)
		return err;

	/* Assign the offset */

	*offset_ptr = entry.Offset;

	return 0;
}

int bmfs_disk_allocate_mebibytes(struct BMFSDisk *disk, uint64_t mebibytes, uint64_t *offset)
{
	return bmfs_disk_allocate(disk, mebibytes * 1024 * 1024, offset);
}

int bmfs_disk_check_signature(struct BMFSDisk *disk)
{

	int err = bmfs_disk_seek(disk, 0, SEEK_SET);
	if (err != 0)
		return 0;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	err = bmfs_header_read(&header, disk);
	if (err != 0)
		return 0;

	if ((header.Signature[0] != 'B')
	 || (header.Signature[1] != 'M')
	 || (header.Signature[2] != 'F')
	 || (header.Signature[3] != 'S')
	 || (header.Signature[4] != 0)
	 || (header.Signature[5] != 0)
	 || (header.Signature[6] != 0)
	 || (header.Signature[7] != 0))
		return 0;

	return 0;
}

int bmfs_disk_bytes(struct BMFSDisk *disk, uint64_t *bytes)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_seek(disk, 0, SEEK_END);
	if (err != 0)
		return err;

	int64_t disk_size;
	err = bmfs_disk_tell(disk, &disk_size);
	if (err != 0)
		return err;

	if (bytes != NULL)
		*bytes = disk_size;

	return 0;
}

int bmfs_disk_mebibytes(struct BMFSDisk *disk, uint64_t *mebibytes)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_bytes(disk, mebibytes);
	if (err != 0)
		return err;

	if (mebibytes != NULL)
		*mebibytes /= (1024 * 1024);

	return 0;
}

int bmfs_disk_blocks(struct BMFSDisk *disk, uint64_t *blocks)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_bytes(disk, blocks);
	if (err != 0)
		return err;

	if (blocks != NULL)
		*blocks /= BMFS_BLOCK_SIZE;

	return 0;
}

int bmfs_disk_create_file(struct BMFSDisk *disk, const char *path, uint64_t mebibytes)
{
	if ((disk == NULL) || (path == NULL))
		return -EFAULT;

	if (mebibytes % 2 != 0)
		mebibytes++;

	uint64_t offset = 0;

	int err = bmfs_disk_allocate_mebibytes(disk, mebibytes, &offset);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_type(&entry, BMFS_TYPE_FILE);
	entry.Offset = offset;
	bmfs_get_current_time(&entry.CreationTime);
	bmfs_get_current_time(&entry.ModificationTime);

	err = create_entry(disk, &entry, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_create_dir(struct BMFSDisk *disk, const char *path)
{
	if ((disk == NULL) || (path == NULL))
		return -EFAULT;

	uint64_t offset = 0;

	int err = bmfs_disk_allocate_mebibytes(disk, 2 /* 2MiB */, &offset);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_type(&entry, BMFS_TYPE_DIRECTORY);
	entry.Offset = offset;
	bmfs_get_current_time(&entry.CreationTime);
	bmfs_get_current_time(&entry.ModificationTime);

	err = create_entry(disk, &entry, path);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_delete_file(struct BMFSDisk *disk, const char *filename)
{
	struct BMFSDir dir;
	int err = bmfs_disk_read_root_dir(disk, &dir);
	if (err != 0)
		return err;

	struct BMFSEntry *entry;
	entry = bmfs_dir_find(&dir, filename);
	if (entry == NULL)
		return -ENOENT;

	entry->Name[0] = 1;

	return bmfs_disk_write_root_dir(disk, &dir);
}

int bmfs_disk_find_file(struct BMFSDisk *disk, const char *filename, struct BMFSEntry *fileentry, uint64_t *entrynumber)
{
	int err;
	struct BMFSDir dir;
	struct BMFSEntry *result;

	err = bmfs_disk_read_root_dir(disk, &dir);
	if (err != 0)
		return err;

	result = bmfs_dir_find(&dir, filename);
	if (result == NULL)
		/* not found */
		return -ENOENT;

	if (fileentry)
		*fileentry = *result;

	if (entrynumber)
		*entrynumber = (result - &dir.Entries[0]) / sizeof(dir.Entries[0]);

	return 0;
}

int bmfs_disk_format(struct BMFSDisk *disk, uint64_t size)
{
	/* Write the file system header. */

	int err = bmfs_disk_seek(disk, 0UL, SEEK_SET);
	if (err != 0)
		return err;

	struct BMFSHeader header;

	bmfs_header_init(&header);

	header.TotalSize = size;

	err = bmfs_header_write(&header, disk);
	if (err != 0)
		return err;

	/* Write the allocation table. */

	err = bmfs_disk_seek(disk, header.TableOffset, SEEK_SET);
	if (err != 0)
		return err;

	for (uint64_t i = 0; i < header.TableEntryCount; i++) {

		struct BMFSTableEntry table_entry;

		bmfs_table_entry_init(&table_entry);

		err = bmfs_table_entry_write(&table_entry, disk);
		if (err != 0)
			return err;
	}

	/* Write the root directory. */

	struct BMFSEntry root;

	bmfs_entry_init(&root);

	bmfs_entry_set_type(&root, BMFS_TYPE_DIRECTORY);

	err = bmfs_disk_allocate(disk, BMFS_BLOCK_SIZE, &root.Offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, header.RootOffset, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_entry_write(&root, disk);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_read(struct BMFSDisk *disk,
              const char *filename,
              void *buf,
              uint64_t len,
              uint64_t off)
{
	struct BMFSEntry entry;

	int err = bmfs_disk_find_file(disk, filename, &entry, NULL);
	if (err != 0)
		return err;

	uint64_t file_offset;

	err = bmfs_entry_get_offset(&entry, &file_offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, file_offset + off, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_read(disk, buf, len, NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_write(struct BMFSDisk *disk,
               const char *filename,
               const void *buf,
               uint64_t len,
               uint64_t off)
{
	struct BMFSEntry entry;

	int err = bmfs_disk_find_file(disk, filename, &entry, NULL);
	if (err != 0)
		return err;

	uint64_t file_offset;

	err = bmfs_entry_get_offset(&entry, &file_offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, file_offset + off, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(disk, buf, len, NULL);
	if (err != 0)
		return err;

	return 0;
}

