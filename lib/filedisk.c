/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/filedisk.h>
#include <bmfs/errno.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4244)
#endif

static int to_bmfs_errno(int libc_errno) {
	switch (libc_errno) {
	case EFAULT:
		return BMFS_EFAULT;
	case EINVAL:
		return BMFS_EINVAL;
	case ENOENT:
		return BMFS_ENOENT;
	case EISDIR:
		return BMFS_EISDIR;
	case EEXIST:
		return BMFS_EEXIST;
	case ENOTDIR:
		return BMFS_ENOTDIR;
	case ENOSPC:
		return BMFS_ENOSPC;
	case ENOSYS:
		return BMFS_ENOSYS;
	case EIO:
		return BMFS_EIO;
	case EPERM:
		return BMFS_EPERM;
	default:
		return BMFS_ENOSYS;
	}
}

static int bmfs_filedisk_seek(void *file_ptr, bmfs_uint64 offset, int whence)
{
	if (file_ptr == BMFS_NULL)
		return BMFS_EFAULT;

#if defined(_MSC_VER)
	if (_fseeki64((FILE *)(file_ptr), offset, whence) != 0)
		return to_bmfs_errno(errno);
#elif defined(__GNUC__)
	if (fseeko((FILE *)(file_ptr), offset, whence) != 0)
		return to_bmfs_errno(errno);
#else
	if (fseek((FILE *)(file_ptr), offset, whence) != 0)
		return to_bmfs_errno(errno);
#endif

	return 0;
}

static int bmfs_filedisk_tell(void *file_ptr, bmfs_uint64 *offset_ptr)
{
	int64_t offset = 0;

	if (file_ptr == BMFS_NULL)
		return BMFS_EFAULT;

#if defined(_MSC_VER)
	offset = (int64_t) _ftelli64((FILE *)(file_ptr));
	if (offset < 0)
		return to_bmfs_errno(errno);
#elif defined(__GNUC__)
	offset = (int64_t) ftello((FILE *)(file_ptr));
	if (offset < 0)
		return to_bmfs_errno(errno);
#else
	offset = (int64_t) ftell((FILE *)(file_ptr));
	if (offset < 0)
		return to_bmfs_errno(errno);
#endif

	if (offset_ptr != BMFS_NULL)
		*offset_ptr = (bmfs_uint64) offset;

	return 0;
}

static int bmfs_filedisk_read(void *file_ptr, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len_ptr)
{
	if ((file_ptr == BMFS_NULL) || (buf == NULL))
		return BMFS_EFAULT;

	size_t read_len = fread(buf, 1, len, (FILE *)(file_ptr));
	if (read_len_ptr != BMFS_NULL)
		*read_len_ptr = read_len;

	return 0;
}

static int bmfs_filedisk_write(void *file_ptr, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len_ptr)
{
	if ((file_ptr == BMFS_NULL) || (buf == NULL))
		return BMFS_EFAULT;

	size_t write_len = fwrite(buf, 1, len, (FILE *)(file_ptr));
	if (write_len_ptr != BMFS_NULL)
		*write_len_ptr = write_len;

	return 0;
}

void bmfs_filedisk_init(struct BMFSFileDisk *filedisk)
{
	filedisk->file = BMFS_NULL;
	bmfs_disk_init(&filedisk->base);
	filedisk->base.disk = BMFS_NULL;
	filedisk->base.seek = bmfs_filedisk_seek;
	filedisk->base.tell = bmfs_filedisk_tell;
	filedisk->base.read = bmfs_filedisk_read;
	filedisk->base.write = bmfs_filedisk_write;
}

void bmfs_filedisk_done(struct BMFSFileDisk *filedisk)
{
	if (filedisk->file != BMFS_NULL)
	{
		fclose(filedisk->file);
		filedisk->file = BMFS_NULL;
	}
}

int bmfs_filedisk_open(struct BMFSFileDisk *filedisk,
                       const char *path,
                       const char *mode)
{
	FILE *file = fopen(path, mode);
	if (file == BMFS_NULL)
		return to_bmfs_errno(errno);

	if (filedisk->file != BMFS_NULL)
		fclose(filedisk->file);

	filedisk->file = file;
	filedisk->base.disk = file;

	return 0;
}
