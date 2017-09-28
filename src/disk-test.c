#include <assert.h>
#include <bmfs/disk.h>
#include <bmfs/limits.h>
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

	if (len > SIZE_MAX)
		len = SIZE_MAX;

	memcpy(buf, &disk->buf[disk->pos], (size_t) len);

	if (read_len != NULL)
		*read_len = len;

	return 0;
}

static int data_write(void *disk_ptr, const void *buf, uint64_t len, uint64_t *write_len)
{
	struct DiskData *disk = (struct DiskData *)(disk_ptr);

	if ((disk->pos + len) > disk->len)
		len = disk->len - disk->pos;

	if (len > SIZE_MAX)
		len = SIZE_MAX;

	memcpy(&disk->buf[disk->pos], buf, (size_t) len);

	if (write_len != NULL)
		*write_len = len;

	return 0;
}

int main(void)
{
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

	/* test allocation */
	uint64_t starting_block = 0;
	assert(bmfs_disk_allocate_bytes(&disk, 1024, &starting_block) == 0);
	assert(starting_block == 1);
	assert(bmfs_disk_allocate_bytes(&disk, BMFS_BLOCK_SIZE * 2, &starting_block) == 0);
	assert(starting_block == 1);
	assert(bmfs_disk_allocate_bytes(&disk, (BMFS_BLOCK_SIZE * 2) + 1, &starting_block) == -ENOSPC);

	/* test the creation of files */
	assert(bmfs_disk_create_file(&disk, "a.txt", 1) == 0);
	struct BMFSDir dir;
	assert(bmfs_disk_read_dir(&disk, &dir) == 0);
	assert(strcmp(dir.Entries[0].FileName, "a.txt") == 0);
	assert(dir.Entries[0].StartingBlock == 1);
	assert(dir.Entries[0].ReservedBlocks == 1);
	assert(dir.Entries[0].FileSize == 0);
	/* make sure buffer is consistent */
	assert(memcmp(&data.buf[4096], "a.txt", 5) == 0);

	assert(bmfs_disk_create_file(&disk, "b.txt", 1) == 0);
	assert(bmfs_disk_read_dir(&disk, &dir) == 0);
	assert(strcmp(dir.Entries[1].FileName, "b.txt") == 0);
	assert(dir.Entries[1].StartingBlock == 2);
	assert(dir.Entries[1].ReservedBlocks == 1);
	assert(dir.Entries[1].FileSize == 0);
	/* make sure buffer is consistent */
	assert(memcmp(&data.buf[4096], "a.txt", 5) == 0);
	assert(memcmp(&data.buf[4096 + 64], "b.txt", 5) == 0);

	/* test to make sure the disk will run out of space */
	assert(bmfs_disk_create_file(&disk, "c.txt", 1) == -ENOSPC);

	/* test to make sure files may be deleted */
	assert(bmfs_disk_delete_file(&disk, "a.txt") == 0);
	assert(bmfs_disk_read_dir(&disk, &dir) == 0);
	assert(dir.Entries[0].FileName[0] == 1);
	assert(strcmp(dir.Entries[1].FileName, "b.txt") == 0);
	/* verify by buffer */
	assert(data.buf[4096] == 1);
	assert(memcmp(&data.buf[4096 + 64], "b.txt", 5) == 0);

	/* test to make sure empty slots can be filled again */
	assert(bmfs_disk_create_file(&disk, "c.txt", 1) == 0);
	assert(bmfs_disk_read_dir(&disk, &dir) == 0);
	assert(strcmp(dir.Entries[0].FileName, "c.txt") == 0);
	assert(strcmp(dir.Entries[1].FileName, "b.txt") == 0);
	assert(dir.Entries[0].StartingBlock == 1);
	assert(dir.Entries[1].StartingBlock == 2);
	/* verify by buffer */
	assert(memcmp(&data.buf[4096], "c.txt", 5) == 0);
	assert(memcmp(&data.buf[4096 + 64], "b.txt", 5) == 0);

	/* test to make sure the same file can't be created */
	assert(bmfs_disk_delete_file(&disk, "b.txt") == 0);
	assert(bmfs_disk_create_file(&disk, "c.txt", 2) == -EEXIST);

	free(data.buf);

	return EXIT_SUCCESS;
}

