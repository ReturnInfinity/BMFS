#include <bmfs/dir.h>
#include <bmfs/errno.h>
#include <bmfs/fs.h>
#include <bmfs/limits.h>
#include <bmfs/ramdisk.h>

#include <stdlib.h>
#include <string.h>

#include "assert.h"

int main(void)
{
	/* Allocate memory for ramdisk */

	size_t buf_size = BMFS_BLOCK_SIZE * 12;

	void *buf = malloc(buf_size);
	bmfs_assert(buf != NULL);

	/* Setup the ramdisk */

	struct BMFSRamdisk ramdisk;

	bmfs_ramdisk_init(&ramdisk);

	int err = bmfs_ramdisk_set_buf(&ramdisk, buf, buf_size);
	bmfs_assert(err == 0);

	/* Setup the file system structure. */

	struct BMFS fs;

	bmfs_init(&fs);

	bmfs_set_disk(&fs, &ramdisk.base);

	/* Format the disk. */

	err = bmfs_format(&fs, buf_size);
	bmfs_assert(err == 0);

	/* Test the creation of directories */

	err = bmfs_create_dir(&fs, "/tmp");
	bmfs_assert(err == 0);

	err = bmfs_create_dir(&fs, "/usr");
	bmfs_assert(err == 0);

	err = bmfs_create_dir(&fs, "/home");
	bmfs_assert(err == 0);

	/* Test the creation of files */

	err = bmfs_create_file(&fs, "/tmp/a.txt");
	bmfs_assert(err == 0);

	err = bmfs_create_file(&fs, "/tmp/b.txt");
	bmfs_assert(err == 0);

	/* Test that the same file can't be created twice. */

	err = bmfs_create_file(&fs, "/tmp/a.txt");
	bmfs_assert(err == BMFS_EEXIST);

	/* Test the creation of subdirectories. */

	err = bmfs_create_dir(&fs, "/usr/local");
	bmfs_assert(err == 0);

	/* Test that files can't be created in
	 * directories that aren't there */

	err = bmfs_create_file(&fs, "/tmp2/b.txt");
	bmfs_assert(err != 0);

	/* Test that the same directory can't be created twice. */

	err = bmfs_create_dir(&fs, "/usr/local");
	bmfs_assert(err == BMFS_EEXIST);

	/* Test that directories can be opened. */

	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	err = bmfs_open_dir(&fs, &dir, "/");

	bmfs_assert(err == 0);

	const struct BMFSEntry *entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "tmp") == 0);
	bmfs_assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "usr") == 0);
	bmfs_assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "home") == 0);
	bmfs_assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry == NULL);

	/* Test that subdirectories can be opened. */

	err = bmfs_open_dir(&fs, &dir, "/usr");
	bmfs_assert(err == 0);

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "local") == 0);
	bmfs_assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry == NULL);

	err = bmfs_open_dir(&fs, &dir, "/tmp");
	bmfs_assert(err == 0);

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "a.txt") == 0);
	bmfs_assert(bmfs_entry_is_file(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry != NULL);
	bmfs_assert(strcmp(entry->Name, "b.txt") == 0);
	bmfs_assert(bmfs_entry_is_file(entry));

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry == NULL);

	/* Test that the empty directories don't contain anything. */

	err = bmfs_open_dir(&fs, &dir, "/home");
	bmfs_assert(err == 0);

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry == NULL);

	err = bmfs_open_dir(&fs, &dir, "/usr/local");
	bmfs_assert(err == 0);

	entry = bmfs_dir_next(&dir);
	bmfs_assert(entry == NULL);

	free(buf);

	return EXIT_SUCCESS;
}
