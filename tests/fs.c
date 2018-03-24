#include <assert.h>
#include <bmfs/ramdisk.h>
#include <bmfs/fs.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

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

	bmfs_set_disk(&fs, &ramdisk.base);

	/* Format the disk. */

	err = bmfs_format(&fs, buf_size);
	assert(err == 0);

	/* Test the creation of directories */

	err = bmfs_create_dir(&fs, "/tmp");
	assert(err == 0);

	err = bmfs_create_dir(&fs, "/usr");
	assert(err == 0);

	err = bmfs_create_dir(&fs, "/home");
	assert(err == 0);

	/* Test the creation of files */

	err = bmfs_create_file(&fs, "/tmp/a.txt");
	assert(err == 0);

	err = bmfs_create_file(&fs, "/tmp/b.txt");
	assert(err == 0);

	/* Test the creation of subdirectories. */

	err = bmfs_create_dir(&fs, "/usr/local");
	assert(err == 0);

	/* Test that files can't be created in
	 * directories that aren't there */

	err = bmfs_create_file(&fs, "/tmp2/b.txt");
	assert(err != 0);

	/* Test that directories can be opened. */

	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	err = bmfs_open_dir(&fs, &dir, "/");

	assert(err == 0);

	const struct BMFSEntry *entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "tmp") == 0);
	assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "usr") == 0);
	assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "home") == 0);
	assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry == NULL);

	/* Test that subdirectories can be opened. */

	err = bmfs_open_dir(&fs, &dir, "/usr");
	assert(err == 0);

	entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "local") == 0);
	assert(bmfs_entry_is_directory(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry == NULL);

	err = bmfs_open_dir(&fs, &dir, "/tmp");
	assert(err == 0);

	entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "a.txt") == 0);
	assert(bmfs_entry_is_file(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry != NULL);
	assert(strcmp(entry->Name, "b.txt") == 0);
	assert(bmfs_entry_is_file(entry));

	entry = bmfs_dir_next(&dir);
	assert(entry == NULL);

	/* Test that the empty directories don't contain anything. */

	err = bmfs_open_dir(&fs, &dir, "/home");
	assert(err == 0);

	entry = bmfs_dir_next(&dir);
	assert(entry == NULL);

	err = bmfs_open_dir(&fs, &dir, "/usr/local");
	assert(err == 0);

	entry = bmfs_dir_next(&dir);
	assert(entry == NULL);

	free(buf);

	return EXIT_SUCCESS;
}
