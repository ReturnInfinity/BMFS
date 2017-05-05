#include "dir.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	assert(bmfs_dir_add_file(&dir, "a.txt") == 0);
	assert(strcmp(dir.Entries[0].FileName, "a.txt") == 0);
	assert(dir.Entries[0].StartingBlock == 1);
	assert(dir.Entries[0].ReservedBlocks == 0);
	assert(dir.Entries[0].FileSize == 0);

	assert(bmfs_dir_add_file(&dir, "b.txt") == 0);
	assert(strcmp(dir.Entries[1].FileName, "b.txt") == 0);
	assert(dir.Entries[1].StartingBlock == 1);
	assert(dir.Entries[1].ReservedBlocks == 0);
	assert(dir.Entries[1].FileSize == 0);

	assert(bmfs_dir_add_file(&dir, "c.txt") == 0);
	assert(strcmp(dir.Entries[2].FileName, "c.txt") == 0);
	assert(dir.Entries[2].StartingBlock == 1);
	assert(dir.Entries[2].ReservedBlocks == 0);
	assert(dir.Entries[2].FileSize == 0);

	struct BMFSEntry *entry;
	entry = bmfs_dir_find(&dir, "b.txt");
	assert(entry == &dir.Entries[1]);

	assert(bmfs_dir_delete_file(&dir, "b.txt") == 0);
	assert(strcmp(dir.Entries[0].FileName, "a.txt") == 0);
	assert(strcmp(dir.Entries[2].FileName, "c.txt") == 0);
	assert(bmfs_entry_is_empty(&dir.Entries[1]));

	assert(bmfs_dir_delete_file(&dir, "missing.txt") == -ENOENT);

	assert(bmfs_dir_add_file(&dir, "a.txt") == -EEXIST);

	/* a.txt */
	dir.Entries[0].StartingBlock = 42;
	/* c.txt */
	dir.Entries[2].StartingBlock = 41;
	assert(bmfs_dir_sort(&dir, bmfs_entry_cmp_by_starting_block) == 0);
	assert(strcmp(dir.Entries[0].FileName, "c.txt") == 0);
	assert(strcmp(dir.Entries[1].FileName, "a.txt") == 0);
	assert(bmfs_entry_is_empty(&dir.Entries[2]));

	assert(bmfs_dir_add_file(&dir, "a.txt.gz") == 0);
	assert(bmfs_dir_sort(&dir, bmfs_entry_cmp_by_filename) == 0);
	assert(strcmp(dir.Entries[0].FileName, "a.txt") == 0);
	assert(strcmp(dir.Entries[1].FileName, "a.txt.gz") == 0);
	assert(strcmp(dir.Entries[2].FileName, "c.txt") == 0);

	return EXIT_SUCCESS;
}

