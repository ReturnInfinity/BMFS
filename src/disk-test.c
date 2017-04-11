#include "disk.h"
#include "limits.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

struct DiskData
{
	char *buf;
	uint64_t len;
	uint64_t pos;
};

static int data_seek(void *disk_ptr, int64_t offset, int whence)
{
	struct DiskData *disk = (struct DiskData *)(disk_ptr);
	if (whence == SEEK_SET)
	{
		if (offset > ((int64_t)(disk->len)))
			disk->pos = disk->len;
		else
			disk->pos = offset;
	}
	else if (whence == SEEK_CUR)
	{
		if ((disk->pos + offset) > disk->len)
			disk->pos = disk->len;
		else if ((((int64_t)(disk->pos)) + offset) < 0)
			disk->pos = 0;
		else
			disk->pos += offset;
	}
	else if (whence == SEEK_END)
	{
		if (offset > ((int64_t)(disk->len)))
			disk->pos = 0;
		else if (offset < 0)
			disk->pos = 0;
		else
			disk->pos = disk->len - offset;
	}

	return 0;
}

static int data_tell(void *disk_ptr, int64_t *offset)
{
	struct DiskData *disk = (struct DiskData *)(disk_ptr);

	*offset = disk->pos;

	return 0;
}

static int data_read(void *disk_ptr, void *buf, uint64_t len, uint64_t *read_len)
{
	struct DiskData *disk = (struct DiskData *)(disk_ptr);

	if ((disk->pos + len) > disk->len)
		len = disk->len - disk->pos;

	memcpy(buf, &disk->buf[disk->pos], len);

	if (read_len != NULL)
		*read_len = len;

	return 0;
}

static int data_write(void *disk_ptr, const void *buf, uint64_t len, uint64_t *write_len)
{
	struct DiskData *disk = (struct DiskData *)(disk_ptr);

	if ((disk->pos + len) > disk->len)
		len = disk->len - disk->pos;

	memcpy(&disk->buf[disk->pos], buf, len);

	if (write_len != NULL)
		*write_len = len;

	return 0;
}

static int zerocmp(const void *a, size_t size)
{
	void *b = calloc(1, size);
	if (b == NULL)
		return -1;

	int cmp_result = memcmp(a, b, size);

	free(b);

	return cmp_result;
}

int main(void){

	struct DiskData data;

	data.buf = malloc(BMFS_MINIMUM_DISK_SIZE);
	if (data.buf == NULL)
		return EXIT_FAILURE;
	data.pos = 0;
	data.len = BMFS_MINIMUM_DISK_SIZE;

	struct BMFSDisk disk;
	disk.disk = &data;
	disk.tell = data_tell;
	disk.seek = data_seek;
	disk.read = data_read;
	disk.write = data_write;

	/* test format function */
	assert(bmfs_disk_format(&disk) == 0);
	assert(memcmp(&data.buf[1024], "BMFS", 4) == 0);
	assert(zerocmp(&data.buf[4096], 4096) == 0);

	/* test allocation */
	uint64_t starting_block = 0;
	assert(bmfs_disk_allocate_bytes(&disk, 1024, &starting_block) == 0);
	assert(starting_block == 1);
	assert(bmfs_disk_allocate_bytes(&disk, BMFS_BLOCK_SIZE * 2, &starting_block) == 0);
	assert(starting_block == 1);
	assert(bmfs_disk_allocate_bytes(&disk, (BMFS_BLOCK_SIZE * 2) + 1, &starting_block) == -ENOSPC);

	free(data.buf);

	return EXIT_SUCCESS;
}

