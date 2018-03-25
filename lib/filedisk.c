/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/filedisk.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4244)
#endif

static int bmfs_filedisk_seek(void *file_ptr, int64_t offset, int whence)
{
	if (file_ptr == NULL)
		return -EFAULT;

#if defined(_MSC_VER)
	if (_fseeki64((FILE *)(file_ptr), offset, whence) != 0)
		return -errno;
#elif defined(__GNUC__)
	if (fseeko((FILE *)(file_ptr), offset, whence) != 0)
		return -errno;
#else
	if (fseek((FILE *)(file_ptr), offset, whence) != 0)
		return -errno;
#endif

	return 0;
}

static int bmfs_filedisk_tell(void *file_ptr, int64_t *offset_ptr)
{
	int64_t offset = 0;

	if (file_ptr == NULL)
		return -EFAULT;

#if defined(_MSC_VER)
	offset = _ftelli64((FILE *)(file_ptr));
	if (offset < 0)
		return -errno;
#elif defined(__GNUC__)
	offset = ftello((FILE *)(file_ptr));
	if (offset < 0)
		return -errno;
#else
	offset = ftell((FILE *)(file_ptr));
	if (offset < 0)
		return -errno;
#endif
	if (offset_ptr != NULL)
		*offset_ptr = offset;

	return 0;
}

static int bmfs_filedisk_read(void *file_ptr, void *buf, uint64_t len, uint64_t *read_len_ptr)
{
	if ((file_ptr == NULL) || (buf == NULL))
		return -EFAULT;

	size_t read_len = fread(buf, 1, len, (FILE *)(file_ptr));
	if (read_len_ptr != NULL)
		*read_len_ptr = read_len;

	return 0;
}

static int bmfs_filedisk_write(void *file_ptr, const void *buf, uint64_t len, uint64_t *write_len_ptr)
{
	if ((file_ptr == NULL) || (buf == NULL))
		return -EFAULT;

	size_t write_len = fwrite(buf, 1, len, (FILE *)(file_ptr));
	if (write_len_ptr != NULL)
		*write_len_ptr = write_len;

	return 0;
}

void bmfs_filedisk_init(struct BMFSFileDisk *filedisk)
{
	filedisk->file = NULL;
	bmfs_disk_init(&filedisk->base);
	filedisk->base.disk = NULL;
	filedisk->base.seek = bmfs_filedisk_seek;
	filedisk->base.tell = bmfs_filedisk_tell;
	filedisk->base.read = bmfs_filedisk_read;
	filedisk->base.write = bmfs_filedisk_write;
}

void bmfs_filedisk_done(struct BMFSFileDisk *filedisk)
{
	if (filedisk->file != NULL)
	{
		fclose(filedisk->file);
		filedisk->file = NULL;
	}
}

int bmfs_filedisk_open(struct BMFSFileDisk *filedisk,
                       const char *path,
                       const char *mode)
{
	FILE *file = fopen(path, mode);
	if (file == NULL)
		return -errno;

	if (filedisk->file != NULL)
		fclose(filedisk->file);

	filedisk->file = file;
	filedisk->base.disk = file;

	return 0;
}
