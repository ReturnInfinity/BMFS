#include <bmfs/fs.h>
#include <bmfs/ramdisk.h>
#include <bmfs/limits.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"

void test_alloc(void)
{
	/* Allocate enough room for the file system
	 * data and three allocations. */

	bmfs_uint64 mem_size = BMFS_BLOCK_SIZE * 6;

	void *mem = malloc(mem_size);
	bmfs_assert(mem != NULL);

	struct BMFSRamdisk ramdisk;

	bmfs_ramdisk_init(&ramdisk);

	bmfs_ramdisk_set_buf(&ramdisk, mem, mem_size);

	/* Setup the file system */

	struct BMFS bmfs;

	bmfs_init(&bmfs);

	bmfs_set_disk(&bmfs, &ramdisk.base);

	int err = bmfs_format(&bmfs, mem_size);
	bmfs_assert(err == 0);

	bmfs_uint64 offset1 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE / 2, &offset1);
	bmfs_assert(err == 0);

	bmfs_uint64 offset2 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE * 2, &offset2);
	bmfs_assert(err == 0);

	bmfs_uint64 offset3 = 0;

	err = bmfs_allocate(&bmfs, BMFS_BLOCK_SIZE / 2, &offset3);
	bmfs_assert(err == 0);

	bmfs_assert(offset2 == (offset1 + BMFS_BLOCK_SIZE));
	bmfs_assert(offset3 == (offset1 + (BMFS_BLOCK_SIZE * 3)));
}

int main(void)
{
	test_alloc();
	return EXIT_SUCCESS;
}
