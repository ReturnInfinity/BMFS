#include <bmfs/stdlib.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4244)
#endif

static int bmfs_disk_file_seek(void *file_ptr, int64_t offset, int whence)
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

static int bmfs_disk_file_tell(void *file_ptr, int64_t *offset_ptr)
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

static int bmfs_disk_file_read(void *file_ptr, void *buf, uint64_t len, uint64_t *read_len_ptr)
{
	if ((file_ptr == NULL)
	 || (buf == NULL))
		return -EFAULT;

	size_t read_len = fread(buf, 1, len, (FILE *)(file_ptr));
	if (read_len_ptr != NULL)
		*read_len_ptr = read_len;

	return 0;
}

static int bmfs_disk_file_write(void *file_ptr, const void *buf, uint64_t len, uint64_t *write_len_ptr)
{
	if ((file_ptr == NULL)
	 || (buf == NULL))
		return -EFAULT;

	size_t write_len = fwrite(buf, 1, len, (FILE *)(file_ptr));
	if (write_len_ptr != NULL)
		*write_len_ptr = write_len;

	return 0;
}

int bmfs_disk_init_file(struct BMFSDisk *disk, FILE *file)
{
	if ((disk == NULL)
	 || (file == NULL))
		return -EFAULT;

	disk->disk = file;
	disk->seek = bmfs_disk_file_seek;
	disk->tell = bmfs_disk_file_tell;
	disk->read = bmfs_disk_file_read;
	disk->write = bmfs_disk_file_write;

	return 0;
}
