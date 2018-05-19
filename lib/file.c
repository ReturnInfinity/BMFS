/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/file.h>

#include <bmfs/disk.h>
#include <bmfs/errno.h>
#include <bmfs/table.h>

#include <stdio.h>

static int file_resize(struct BMFSFile *file,
                       bmfs_uint64 size)
{
	if (file->Table == BMFS_NULL)
		return BMFS_EFAULT;

	bmfs_uint64 offset = file->Entry.Offset;

	int err = bmfs_table_realloc(file->Table, size, &offset);
	if (err != 0)
		return err;

	file->Entry.Offset = offset;
	file->Entry.Size = size;

	printf("New Offset : %llu\n", offset);
	printf("New Size   : %llu\n", size);

	return 0;
}

static bmfs_bool file_can_write(const struct BMFSFile *file)
{
	if ((file->Mode == BMFS_FILE_MODE_WRITE)
	 || (file->Mode == BMFS_FILE_MODE_RW))
	{
		return BMFS_TRUE;
	}

	return BMFS_FALSE;
}

void bmfs_file_init(struct BMFSFile *file)
{
	bmfs_entry_init(&file->Entry);
	file->Disk = BMFS_NULL;
	file->CurrentPosition = 0;
	file->ReservedSize = 0;
	file->Table = BMFS_NULL;
	file->Mode = BMFS_FILE_MODE_READ;
}

void bmfs_file_close(struct BMFSFile *file)
{
	/* Only update information if writing
	 * is enabled. */
	if (!file_can_write(file))
		return;

	bmfs_entry_save(&file->Entry, file->Disk);
}

void bmfs_file_truncate(struct BMFSFile *file)
{
	if (file_can_write(file))
	{
		file->CurrentPosition = 0;
		file->Entry.Size = 0;
	}
}

void bmfs_file_set_disk(struct BMFSFile *file,
                        struct BMFSDisk *disk)
{
	if ((file != BMFS_NULL) && (disk != BMFS_NULL))
	{
		file->Disk = disk;
	}
}

void bmfs_file_set_mode(struct BMFSFile *file,
                        enum BMFSFileMode mode)
{
	if (file != BMFS_NULL)
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

int bmfs_file_eof(const struct BMFSFile *file)
{
	if (file->CurrentPosition >= file->Entry.Size)
		return 1;
	else
		return 0;
}

int bmfs_file_read(struct BMFSFile *file,
                   void *buf,
                   bmfs_uint64 buf_size,
                   bmfs_uint64 *read_result_ptr)
{
	if ((file->Mode != BMFS_FILE_MODE_READ)
	 && (file->Mode != BMFS_FILE_MODE_RW))
		return BMFS_EINVAL;

	int err = bmfs_file_seek(file, file->CurrentPosition, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	if ((file->CurrentPosition + buf_size) > file->Entry.Size)
		buf_size = file->Entry.Size - file->CurrentPosition;

	bmfs_uint64 read_result = 0;

	err = bmfs_disk_read(file->Disk, buf, buf_size, &read_result);
	if (err != 0)
		return err;

	file->CurrentPosition += read_result;

	if (read_result_ptr != BMFS_NULL)
		*read_result_ptr = read_result;

	return 0;
}

int bmfs_file_write(struct BMFSFile *file,
                    const void *buf,
                    bmfs_uint64 buf_size,
                    bmfs_uint64 *write_result_ptr)
{
	if ((file->Mode != BMFS_FILE_MODE_WRITE)
	 && (file->Mode != BMFS_FILE_MODE_RW))
		return BMFS_EINVAL;

	bmfs_uint64 new_size = file->CurrentPosition + buf_size;

	if (new_size > file->Entry.Size)
	{
		int err = file_resize(file, new_size);
		if (err != 0)
			return err;
	}

	int err = bmfs_file_seek(file, file->CurrentPosition, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 write_result = 0;

	err = bmfs_disk_write(file->Disk, buf, buf_size, &write_result);
	if (err != 0)
		return err;

	file->CurrentPosition += write_result;

	if (write_result_ptr != BMFS_NULL)
		*write_result_ptr = write_result;

	if (file->Entry.Size < file->CurrentPosition)
		file->Entry.Size = file->CurrentPosition;

	return 0;
}

int bmfs_file_seek(struct BMFSFile *file,
                   bmfs_uint64 pos,
                   int whence)
{
	bmfs_uint64 next_pos = 0;

	if (whence == BMFS_SEEK_SET)
	{
		/* TODO : allocate more space for large seeks */
		if (pos > file->Entry.Size)
			return BMFS_EINVAL;

		next_pos = pos;
	}
	else if (whence == BMFS_SEEK_END)
	{
		if (pos > file->Entry.Size)
			return BMFS_EINVAL;

		next_pos = file->Entry.Size - pos;
	}
	else
	{
		return BMFS_EINVAL;
	}

	file->CurrentPosition = next_pos;

	int err = bmfs_disk_seek(file->Disk, file->Entry.Offset + file->CurrentPosition, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	return 0;
}

int bmfs_file_tell(struct BMFSFile *file,
                   bmfs_uint64 *pos)
{
	if ((file == BMFS_NULL) || (pos == BMFS_NULL))
		return BMFS_EFAULT;

	*pos = file->CurrentPosition;

	return 0;
}
