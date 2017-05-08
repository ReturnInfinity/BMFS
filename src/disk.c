/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include <bmfs/disk.h>
#include <bmfs/limits.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

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

/* public functions */

int bmfs_disk_read_dir(struct BMFSDisk *disk, struct BMFSDir *dir)
{
	int err = bmfs_disk_seek(disk, 4096, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_read(disk, dir->Entries, sizeof(dir->Entries), NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_write_dir(struct BMFSDisk *disk, const struct BMFSDir *dir)
{
	int err = bmfs_disk_seek(disk, 4096, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(disk, dir->Entries, sizeof(dir->Entries), NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_allocate_bytes(struct BMFSDisk *disk, size_t bytes, size_t *starting_block)
{
	if ((disk == NULL)
	 || (starting_block == NULL))
		return -EFAULT;

	/* make bytes % BMFS_BLOCK_SIZE == 0
	 * by rounding up */
	if ((bytes % BMFS_BLOCK_SIZE) != 0)
		bytes += BMFS_BLOCK_SIZE - (bytes % BMFS_BLOCK_SIZE);

	struct BMFSDir dir;

	int err = bmfs_disk_read_dir(disk, &dir);
	if (err != 0)
		return err;

	err = bmfs_dir_sort(&dir, bmfs_entry_cmp_by_starting_block);
	if (err != 0)
		return err;

	size_t total_blocks;
	err = bmfs_disk_blocks(disk, &total_blocks);
	if (err != 0)
		return err;
	else if (total_blocks == 0)
		return -ENOSPC;

	size_t prev_block = 1;
	size_t next_block = total_blocks;

	for (size_t i = 0; i < 64; i++)
	{
		struct BMFSEntry *entry = &dir.Entries[i];
		if (!(bmfs_entry_is_empty(entry))
		 && !(bmfs_entry_is_terminator(entry)))
			next_block = entry->StartingBlock;

		size_t blocks_between = next_block - prev_block;
		if ((blocks_between * BMFS_BLOCK_SIZE) >= bytes)
		{
			/* found a spot between entries */
			*starting_block = prev_block;
			return 0;
		}

		prev_block = next_block + entry->ReservedBlocks;
		next_block = total_blocks;
	}

	return -ENOSPC;
}

int bmfs_disk_allocate_mebibytes(struct BMFSDisk *disk, size_t mebibytes, size_t *starting_block)
{
	return bmfs_disk_allocate_bytes(disk, mebibytes * 1024 * 1024, starting_block);
}

int bmfs_disk_bytes(struct BMFSDisk *disk, size_t *bytes)
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

int bmfs_disk_mebibytes(struct BMFSDisk *disk, size_t *mebibytes)
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

int bmfs_disk_blocks(struct BMFSDisk *disk, size_t *blocks)
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

int bmfs_disk_create_file(struct BMFSDisk *disk, const char *filename, size_t mebibytes)
{
	if ((disk == NULL)
	 || (filename == NULL))
		return -EFAULT;

	if (mebibytes % 2 != 0)
		mebibytes++;

	size_t starting_block;
	int err = bmfs_disk_allocate_mebibytes(disk, mebibytes, &starting_block);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_file_name(&entry, filename);
	bmfs_entry_set_starting_block(&entry, starting_block);
	bmfs_entry_set_reserved_blocks(&entry, mebibytes / 2);

	struct BMFSDir dir;

	err = bmfs_disk_read_dir(disk, &dir);
	if (err != 0)
		return err;

	err = bmfs_dir_add(&dir, &entry);
	if (err != 0)
		return err;

	err = bmfs_disk_write_dir(disk, &dir);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_delete_file(struct BMFSDisk *disk, const char *filename)
{
	struct BMFSDir dir;
	int err = bmfs_disk_read_dir(disk, &dir);
	if (err != 0)
		return err;

	struct BMFSEntry *entry;
	entry = bmfs_dir_find(&dir, filename);
	if (entry == NULL)
		return -ENOENT;

	entry->FileName[0] = 1;

	return bmfs_disk_write_dir(disk, &dir);
}

int bmfs_disk_find_file(struct BMFSDisk *disk, const char *filename, struct BMFSEntry *fileentry, int *entrynumber)
{
	int err;
	struct BMFSDir dir;
	struct BMFSEntry *result;

	err = bmfs_disk_read_dir(disk, &dir);
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

int bmfs_disk_check_tag(struct BMFSDisk *disk)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_seek(disk, 1024, SEEK_SET);
	if (err != 0)
		return err;

	char tag[4];
	err = bmfs_disk_read(disk, tag, 4, NULL);
	if (err != 0)
		return err;
	else if ((tag[0] != 'B')
	      || (tag[1] != 'M')
	      || (tag[2] != 'F')
	      || (tag[3] != 'S'))
		return -EINVAL;

	return 0;
}

int bmfs_disk_write_tag(struct BMFSDisk *disk)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_seek(disk, 1024, SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(disk, "BMFS", 4, NULL);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_disk_format(struct BMFSDisk *disk)
{
	int err = bmfs_disk_write_tag(disk);
	if (err != 0)
		return err;

	struct BMFSDir dir;
	bmfs_dir_init(&dir);

	err = bmfs_disk_write_dir(disk, &dir);
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

