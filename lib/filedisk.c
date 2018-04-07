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

static int bmfs_filedisk_seek(void *filedisk_ptr, bmfs_uint64 offset, int whence)
{
	if (filedisk_ptr == BMFS_NULL)
		return BMFS_EFAULT;

	struct BMFSFileDisk *filedisk = (struct BMFSFileDisk *) filedisk_ptr;

	if (filedisk->file == NULL)
		return BMFS_EFAULT;

	if (whence == BMFS_SEEK_SET)
	{
		offset += filedisk->offset;
		whence = SEEK_SET;
	}
	else if (whence == BMFS_SEEK_END)
	{
		/* TODO : ensure that the offset
		 * is not being accessed. Check that
		 * the offset is not greater than the
		 * disk size. */
		whence = SEEK_END;
	}
	else
	{
		return BMFS_EINVAL;
	}

#if defined(_MSC_VER)
	if (_fseeki64(filedisk->file, offset, whence) != 0)
		return to_bmfs_errno(errno);
#elif defined(__GNUC__)
	if (fseeko(filedisk->file, offset, whence) != 0)
		return to_bmfs_errno(errno);
#else
	if (fseek(filedisk->file, offset, whence) != 0)
		return to_bmfs_errno(errno);
#endif

	return 0;
}

static int bmfs_filedisk_tell(void *filedisk_ptr, bmfs_uint64 *offset_ptr)
{
	if (filedisk_ptr == BMFS_NULL)
		return BMFS_EFAULT;

	struct BMFSFileDisk *filedisk = (struct BMFSFileDisk *) filedisk_ptr;

	if (filedisk->file == NULL)
		return BMFS_EFAULT;

	int64_t offset = 0;

#if defined(_MSC_VER)
	offset = (int64_t) _ftelli64(filedisk->file);
	if (offset < 0)
		return to_bmfs_errno(errno);
#elif defined(__GNUC__)
	offset = (int64_t) ftello(filedisk->file);
	if (offset < 0)
		return to_bmfs_errno(errno);
#else
	offset = (int64_t) ftell(filedisk->file);
	if (offset < 0)
		return to_bmfs_errno(errno);
#endif

	offset -= (int64_t) filedisk->offset;

	if (offset_ptr != BMFS_NULL)
		*offset_ptr = (bmfs_uint64) offset;

	return 0;
}

static int bmfs_filedisk_read(void *filedisk_ptr, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len_ptr)
{
	if ((filedisk_ptr == BMFS_NULL) || (buf == NULL))
		return BMFS_EFAULT;

	struct BMFSFileDisk *filedisk = (struct BMFSFileDisk *) filedisk_ptr;

	if (filedisk->file == NULL)
		return BMFS_EFAULT;

	size_t read_len = fread(buf, 1, len, filedisk->file);
	if (read_len_ptr != BMFS_NULL)
		*read_len_ptr = read_len;

	return 0;
}

static int bmfs_filedisk_write(void *filedisk_ptr, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len_ptr)
{
	if ((filedisk_ptr == BMFS_NULL) || (buf == NULL))
		return BMFS_EFAULT;

	struct BMFSFileDisk *filedisk = (struct BMFSFileDisk *) filedisk_ptr;

	if (filedisk->file == NULL)
		return BMFS_EFAULT;

	size_t write_len = fwrite(buf, 1, len, filedisk->file);
	if (write_len_ptr != BMFS_NULL)
		*write_len_ptr = write_len;

	return 0;
}

void bmfs_filedisk_init(struct BMFSFileDisk *filedisk)
{
	filedisk->file = BMFS_NULL;
	bmfs_disk_init(&filedisk->base);
	filedisk->base.disk = filedisk;
	filedisk->base.seek = bmfs_filedisk_seek;
	filedisk->base.tell = bmfs_filedisk_tell;
	filedisk->base.read = bmfs_filedisk_read;
	filedisk->base.write = bmfs_filedisk_write;
	filedisk->offset = 512;
}

void bmfs_filedisk_done(struct BMFSFileDisk *filedisk)
{
	if (filedisk->file != BMFS_NULL)
	{
		fclose(filedisk->file);
		filedisk->file = BMFS_NULL;
	}
}

void bmfs_filedisk_set_offset(struct BMFSFileDisk *filedisk,
                              bmfs_uint64 offset) {

	filedisk->offset = offset;
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

	return 0;
}
