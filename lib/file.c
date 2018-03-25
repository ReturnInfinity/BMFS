/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/file.h>

#include <bmfs/disk.h>

#include <errno.h>

void bmfs_file_init(struct BMFSFile *file)
{
	bmfs_entry_init(&file->Entry);
	file->Disk = NULL;
	file->CurrentPosition = 0;
	file->ReservedSize = 0;
	file->Mode = BMFS_FILE_MODE_READ;
}

void bmfs_file_set_disk(struct BMFSFile *file,
                        struct BMFSDisk *disk)
{
	if ((file != NULL) && (disk != NULL))
	{
		file->Disk = disk;
	}
}

void bmfs_file_set_mode(struct BMFSFile *file,
                        enum BMFSFileMode mode)
{
	if (file != NULL)
		file->Mode = mode;
}

int bmfs_file_import(struct BMFSFile *file)
{
	int err = bmfs_entry_read(&file->Entry, file->Disk);
	if (err != 0)
		return err;

	file->CurrentPosition = 0;

	return 0;
}

int bmfs_file_read(struct BMFSFile *file,
                   void *buf,
                   uint64_t buf_size,
                   uint64_t *read_result)
{
	int err = bmfs_file_seek(file, file->CurrentPosition, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_read(file->Disk, buf, buf_size, read_result);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_file_write(struct BMFSFile *file,
                    const void *buf,
                    uint64_t buf_size,
                    uint64_t *write_result)
{
	int err = bmfs_file_seek(file, file->CurrentPosition, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(file->Disk, buf, buf_size, write_result);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_file_seek(struct BMFSFile *file,
                   int64_t pos,
                   int whence)
{
	uint64_t next_pos = 0;

	if (whence == BMFS_SEEK_SET)
	{
		/* TODO : allocate more space for large seeks */
		if (pos > ((int64_t) file->Entry.Size))
			return -EINVAL;

		next_pos = (uint64_t) pos;
	}
	else if (whence == BMFS_SEEK_END)
	{
		if ((pos * -1) > ((int64_t) file->Entry.Size))
			return -EINVAL;

		next_pos = (uint64_t)(((int64_t) file->Entry.Size) - pos);
	}
	else
	{
		return -EINVAL;
	}

	file->CurrentPosition = next_pos;

	return 0;
}

int bmfs_file_tell(struct BMFSFile *file,
                   uint64_t *pos)
{
	if ((file == NULL) || (pos == NULL))
		return -EFAULT;

	*pos = file->CurrentPosition;

	return 0;
}
