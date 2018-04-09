/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/ramdisk.h>

#include <bmfs/errno.h>
#include <bmfs/limits.h>
#include <bmfs/types.h>

static void bmfs_memcpy(void *dst, const void *src, bmfs_uint64 size)
{
	unsigned char *dst8 = (unsigned char *) dst;
	const unsigned char *src8 = (const unsigned char *) src;
	for (bmfs_uint64 i = 0; i < size; i++)
		dst8[i] = src8[i];
}

static int ramdisk_seek(void *disk_ptr, bmfs_uint64 offset, int whence)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);
	if (whence == BMFS_SEEK_SET)
	{
		if (offset > disk->buf_size)
			disk->buf_pos = disk->buf_size;
		else
			disk->buf_pos = offset;
	}
	else if (whence == BMFS_SEEK_END)
	{
		if (offset > disk->buf_size)
			disk->buf_pos = 0;
		else
			disk->buf_pos = disk->buf_size - offset;
	}
	else
	{
		return BMFS_EINVAL;
	}

	return 0;
}

static int ramdisk_tell(void *disk_ptr, bmfs_uint64 *offset)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	*offset = disk->buf_pos;

	return 0;
}

static int ramdisk_read(void *disk_ptr, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	if ((disk->buf_pos + len) > disk->buf_size)
		len = disk->buf_size - disk->buf_pos;

	bmfs_memcpy(buf, &disk->buf[disk->buf_pos], len);

	if (read_len != BMFS_NULL)
		*read_len = len;

	disk->buf_pos += len;

	return 0;
}

static int ramdisk_write(void *disk_ptr, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	if ((disk->buf_pos + len) > disk->buf_size)
		len = disk->buf_size - disk->buf_pos;

	bmfs_memcpy(&disk->buf[disk->buf_pos], buf, len);

	if (write_len != BMFS_NULL)
		*write_len = len;

	disk->buf_pos += len;

	return 0;
}

void bmfs_ramdisk_init(struct BMFSRamdisk *ramdisk)
{
	bmfs_disk_init(&ramdisk->base);
	ramdisk->base.DiskPtr = ramdisk;
	ramdisk->base.seek = ramdisk_seek;
	ramdisk->base.tell = ramdisk_tell;
	ramdisk->base.read = ramdisk_read;
	ramdisk->base.write = ramdisk_write;
	ramdisk->buf = BMFS_NULL;
	ramdisk->buf_size = 0;
	ramdisk->buf_pos = 0;
}

void bmfs_ramdisk_done(struct BMFSRamdisk *ramdisk)
{
	(void) ramdisk;
}

int bmfs_ramdisk_set_buf(struct BMFSRamdisk *ramdisk,
                         void *buf, bmfs_uint64 buf_size)
{
	if (buf == BMFS_NULL)
		return BMFS_EFAULT;

	ramdisk->buf = (unsigned char *) buf;
	ramdisk->buf_size = buf_size;
	/* In case a buffer was already set,
	 * reset the position to zero. */
	ramdisk->buf_pos = 0;

	return 0;
}
