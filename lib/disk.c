/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/disk.h>

#include <bmfs/errno.h>
#include <bmfs/limits.h>

/* disk wrapper functions */

int bmfs_disk_seek(struct BMFSDisk *disk, bmfs_uint64 offset, int whence)
{
	if ((disk == BMFS_NULL)
	 || (disk->seek == BMFS_NULL))
		return BMFS_EFAULT;

	return disk->seek(disk->disk, offset, whence);
}

int bmfs_disk_tell(struct BMFSDisk *disk, bmfs_uint64 *offset)
{
	if ((disk == BMFS_NULL)
	 || (disk->tell == BMFS_NULL))
		return BMFS_EFAULT;

	return disk->tell(disk->disk, offset);
}

int bmfs_disk_read(struct BMFSDisk *disk, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len)
{
	if ((disk == BMFS_NULL)
	 || (disk->read == BMFS_NULL))
		return BMFS_EFAULT;

	return disk->read(disk->disk, buf, len, read_len);
}

int bmfs_disk_write(struct BMFSDisk *disk, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len)
{
	if ((disk == BMFS_NULL)
	 || (disk->write == BMFS_NULL))
		return BMFS_EFAULT;

	return disk->write(disk->disk, buf, len, write_len);
}

/* public functions */

void bmfs_disk_init(struct BMFSDisk *disk)
{
	disk->disk = BMFS_NULL;
	disk->done = BMFS_NULL;
	disk->seek = BMFS_NULL;
	disk->tell = BMFS_NULL;
	disk->read = BMFS_NULL;
	disk->write = BMFS_NULL;
}

void bmfs_disk_done(struct BMFSDisk *disk)
{
	if (disk->done != BMFS_NULL)
		disk->done(disk->disk);
}
