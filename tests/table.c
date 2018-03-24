#include <bmfs/fs.h>
#include <bmfs/ramdisk.h>
#include <bmfs/limits.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_alloc(void)
{
	/* Allocate enough room for the file system
	 * data and three allocations. */

	size_t mem_size = BMFS_BLOCK_SIZE * 6;

	void *mem = malloc(mem_size);
	assert(mem != NULL);

	struct BMFSRamdisk ramdisk;

	bmfs_ramdisk_init(&ramdisk);

	bmfs_ramdisk_set_buf(&ramdisk, mem, mem_size);

	/* Setup the file system */

	struct BMFS bmfs;

	bmfs_init(&bmfs);

	bmfs_set_disk(&bmfs, &ramdisk.base);

	int err = bmfs_format(&bmfs, mem_size);
	assert(err == 0);

	uint64_t offset1 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE / 2, &offset1);
	assert(err == 0);

	uint64_t offset2 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE * 2, &offset2);
	assert(err == 0);

	uint64_t offset3 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE / 2, &offset3);
	assert(err == 0);

	assert(offset2 == (offset1 + BMFS_BLOCK_SIZE));
	assert(offset3 == (offset1 + (BMFS_BLOCK_SIZE * 3)));
}

int main(void)
{
	test_alloc();
	return EXIT_SUCCESS;
}
