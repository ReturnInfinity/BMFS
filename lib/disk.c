/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/disk.h>

#include <errno.h>

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

int bmfs_disk_bytes(struct BMFSDisk *disk, uint64_t *bytes)
{
	if (disk == NULL)
		return -EFAULT;

	int err = bmfs_disk_seek(disk, 0, BMFS_SEEK_END);
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
