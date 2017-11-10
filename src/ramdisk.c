/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#include <bmfs/ramdisk.h>

#include <bmfs/limits.h>

#include <errno.h>
#include <stdio.h>

static void bmfs_memcpy(void *dst, const void *src, uint64_t size)
{
	unsigned char *dst8 = (unsigned char *) dst;
	const unsigned char *src8 = (const unsigned char *) src;
	for (uint64_t i = 0; i < size; i++)
		dst8[i] = src8[i];
}

static void ramdisk_done(void *ramdisk_ptr)
{
	/* nothing to do here */
	(void) ramdisk_ptr;
}

static int ramdisk_seek(void *disk_ptr, int64_t offset, int whence)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);
	if (whence == SEEK_SET)
	{
		if (offset > ((int64_t)(disk->buf_size)))
			disk->buf_pos = disk->buf_size;
		else
			disk->buf_pos = offset;
	}
	else if (whence == SEEK_CUR)
	{
		if ((disk->buf_pos + offset) > disk->buf_size)
			disk->buf_pos = disk->buf_size;
		else if ((((int64_t)(disk->buf_pos)) + offset) < 0)
			disk->buf_pos = 0;
		else
			disk->buf_pos += offset;
	}
	else if (whence == SEEK_END)
	{
		if (offset > ((int64_t)(disk->buf_size)))
			disk->buf_pos = 0;
		else if (offset < 0)
			disk->buf_pos = 0;
		else
			disk->buf_pos = disk->buf_size - offset;
	}

	return 0;
}

static int ramdisk_tell(void *disk_ptr, int64_t *offset)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	*offset = disk->buf_pos;

	return 0;
}

static int ramdisk_read(void *disk_ptr, void *buf, uint64_t len, uint64_t *read_len)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	if ((disk->buf_pos + len) > disk->buf_size)
		len = disk->buf_size - disk->buf_pos;

	if (len > SIZE_MAX)
		len = SIZE_MAX;

	bmfs_memcpy(buf, &disk->buf[disk->buf_pos], (size_t) len);

	if (read_len != NULL)
		*read_len = len;

	disk->buf_pos += len;

	return 0;
}

static int ramdisk_write(void *disk_ptr, const void *buf, uint64_t len, uint64_t *write_len)
{
	struct BMFSRamdisk *disk = (struct BMFSRamdisk *)(disk_ptr);

	if ((disk->buf_pos + len) > disk->buf_size)
		len = disk->buf_size - disk->buf_pos;

	if (len > SIZE_MAX)
		len = SIZE_MAX;

	bmfs_memcpy(&disk->buf[disk->buf_pos], buf, (size_t) len);

	if (write_len != NULL)
		*write_len = len;

	disk->buf_pos += len;

	return 0;
}

void bmfs_ramdisk_init(struct BMFSRamdisk *ramdisk)
{
	bmfs_disk_init(&ramdisk->base);
	ramdisk->base.disk = ramdisk;
	ramdisk->base.done = ramdisk_done;
	ramdisk->base.seek = ramdisk_seek;
	ramdisk->base.tell = ramdisk_tell;
	ramdisk->base.read = ramdisk_read;
	ramdisk->base.write = ramdisk_write;
	ramdisk->buf = NULL;
	ramdisk->buf_size = 0;
	ramdisk->buf_pos = 0;
}

void bmfs_ramdisk_done(struct BMFSRamdisk *ramdisk)
{
	bmfs_disk_done(&ramdisk->base);
}

int bmfs_ramdisk_set_buf(struct BMFSRamdisk *ramdisk,
                         void *buf, uint64_t buf_size)
{
	if (buf_size < BMFS_MINIMUM_DISK_SIZE)
		return -ENOMEM;
	else if (buf == NULL)
		return -EFAULT;

	ramdisk->buf = (unsigned char *) buf;
	ramdisk->buf_size = buf_size;
	/* In case a buffer was already set,
	 * reset the position to zero. */
	ramdisk->buf_pos = 0;

	return 0;
}
