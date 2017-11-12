#include <bmfs/explorer.h>
#include <bmfs/ramdisk.h>
#include <bmfs/limits.h>
#include <bmfs/path.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	uint64_t ramdisk_size = BMFS_MINIMUM_DISK_SIZE;

	void *ramdisk_buf = calloc(1, ramdisk_size);
	assert(ramdisk_buf != NULL);

	struct BMFSRamdisk ramdisk;
	bmfs_ramdisk_init(&ramdisk);
	int err = bmfs_ramdisk_set_buf(&ramdisk, ramdisk_buf, ramdisk_size);
	assert(err == 0);

	err = bmfs_disk_format(&ramdisk.base);
	assert(err == 0);

	err = bmfs_disk_create_dir(&ramdisk.base, "usr");
	assert(err == 0);

	struct BMFSExplorer explorer;

	bmfs_explorer_init(&explorer);

	explorer.Disk = ramdisk.base;

	err = bmfs_explorer_read(&explorer);
	assert(err == 0);

	/* not a directory, this should fail */
	struct BMFSPath bad_path;
	bmfs_path_init(&bad_path);
	bad_path.String = "/does-not-exist";
	bad_path.Length = strlen(bad_path.String);
	err = bmfs_explorer_move(&explorer, &bad_path);
	assert(err != 0);

	struct BMFSPath usr_path;
	bmfs_path_init(&usr_path);
	usr_path.String = "/usr";
	usr_path.Length = strlen(usr_path.String);
	err = bmfs_explorer_move(&explorer, &usr_path);
	assert(err == 0);
	err = bmfs_explorer_create_file(&explorer, "a.txt");
	assert(err == 0);

	/* '/usr'   start: (usr.StartingBlock * BMFS_BLOCK_SIZE)
	 * '/usr/a' start: (usr.StartingBlock * BMFS_BLOCK_SIZE)[0].StartingBlock * BMFS_BLOCK_SIZE
	 * */

	bmfs_ramdisk_done(&ramdisk);

	free(ramdisk_buf);

	return EXIT_SUCCESS;
}

