#include <assert.h>
#include <bmfs/ramdisk.h>
#include <bmfs/fs.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	/* Allocate memory for ramdisk */

	size_t buf_size = BMFS_BLOCK_SIZE * 12;

	void *buf = malloc(buf_size);
	assert(buf != NULL);

	/* Setup the ramdisk */

	struct BMFSRamdisk ramdisk;

	bmfs_ramdisk_init(&ramdisk);

	int err = bmfs_ramdisk_set_buf(&ramdisk, buf, buf_size);
	assert(err == 0);

	/* Setup the file system structure. */

	struct BMFS fs;

	bmfs_init(&fs);

	bmfs_set_disk(&fs, ramdisk.base);

	/* Format the disk. */

	err = bmfs_format(disk, buf_size);
	assert(err == 0);

	/* Test the creation of directories */

	err = bmfs_create_dir(disk, "/tmp");
	assert(err == 0);

	/* Test the creation of files */

	err = bmfs_create_file(disk, "/tmp/a.txt", 1);
	printf("err: %s\n", strerror(-err));
	assert(err == 0);

	/* Test that files can't be created in
	 * directories that aren't there */

	err = bmfs_create_file(disk, "/tmp2/b.txt", 1);
	assert(err != 0);

	free(buf);

	return EXIT_SUCCESS;
}
