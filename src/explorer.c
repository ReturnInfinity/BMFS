/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include <bmfs/explorer.h>

#include <bmfs/limits.h>
#include <bmfs/path.h>

#include <errno.h>
#include <stdio.h>

// TODO : find a better way to do this.
static int enter_subdir(struct BMFSExplorer *explorer,
                        const struct BMFSPath *path)
{
	char dirname[33];
	/* dirname index */
	uint64_t i = 0;
	/* path index */
	uint64_t j = 0;
	/* skip leading '/' in path */
	while ((j < path->Length) && path->String[j] == '/')
		j++;
	/* copy the directory name */
	while ((j < path->Length) && (i < 32))
		dirname[i++] = path->String[j++];
	/* add the null terminator */
	dirname[i] = 0;

	for (uint64_t i = 0; i < 64; i++)
	{
		struct BMFSEntry *entry = &explorer->CurrentDirectory.Entries[i];

		if (bmfs_entry_is_terminator(entry))
			break;
		else if (bmfs_entry_is_empty(entry))
			continue;
		else if (!bmfs_entry_is_directory(entry))
			continue;
		else if (bmfs_entry_cmp_filename(entry, dirname) != 0)
			continue;

		/* at this point, the entry was found */

		/* calculate where the disk should seek
		 * to, in order to read the directory */

		uint64_t dir_offset = 0;
		dir_offset += entry->StartingBlock;
		dir_offset *= BMFS_BLOCK_SIZE;

		explorer->CurrentDirectoryOffset = dir_offset;

		return bmfs_explorer_read(explorer);
	}

	return -ENOENT;
}

void bmfs_explorer_init(struct BMFSExplorer *explorer)
{
	bmfs_disk_init(&explorer->Disk);
	bmfs_dir_init(&explorer->CurrentDirectory);
	explorer->CurrentDirectoryOffset = 4096;
}

void bmfs_explorer_done(struct BMFSExplorer *explorer)
{
	bmfs_disk_done(&explorer->Disk);
}

int bmfs_explorer_create(struct BMFSExplorer *explorer,
                         const struct BMFSEntry *entry)
{
	int err = bmfs_dir_add(&explorer->CurrentDirectory, entry);
	if (err != 0)
		return err;

	return bmfs_explorer_write(explorer);
}

int bmfs_explorer_create_file(struct BMFSExplorer *explorer,
                              const char *filename)
{
	uint64_t starting_block = 0;

	int err = bmfs_disk_allocate_mebibytes(&explorer->Disk, 2, &starting_block);
	if (err != 0)
		return err;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_file_name(&entry, filename);
	bmfs_entry_set_starting_block(&entry, starting_block);
	bmfs_entry_set_reserved_blocks(&entry, 1);

	return bmfs_explorer_create(explorer, &entry);
}

int bmfs_explorer_read(struct BMFSExplorer *explorer)
{
	/* seek to directory location */

	int err = bmfs_disk_seek(&explorer->Disk, (int64_t) explorer->CurrentDirectoryOffset, SEEK_SET);
	if (err != 0)
		return err;

	/* read the directory */

	uint64_t read_size = 64 * sizeof(struct BMFSEntry);

	uint64_t actual_read_size = 0;

	err = bmfs_disk_read(&explorer->Disk,
	                     &explorer->CurrentDirectory.Entries[0],
	                     read_size,
	                     &actual_read_size);
	if (err != 0)
		return err;
	else if (actual_read_size != read_size)
		/* TODO : handle this without error */
		return -EIO;

	return 0;
}

int bmfs_explorer_write(struct BMFSExplorer *explorer)
{
	int err = bmfs_disk_seek(&explorer->Disk, (int64_t) explorer->CurrentDirectoryOffset, SEEK_SET);
	if (err != 0)
		return err;

	uint64_t write_size = 64 * sizeof(struct BMFSEntry);

	uint64_t actual_write_size = 0;

	err = bmfs_disk_read(&explorer->Disk,
	                     &explorer->CurrentDirectory.Entries[0],
	                     write_size,
	                     &actual_write_size);
	if (err != 0)
		return err;
	else if (actual_write_size != write_size)
		/* TODO : handle this without error */
		return -EIO;

	return 0;
}

int bmfs_explorer_move(struct BMFSExplorer *explorer,
                       const struct BMFSPath *dir_path)
{

	/* This function works by breaking of the parent
	 * directory of the path, entering it, and repeating
	 * until the path no longer has a parent. For example:
	 *
	 *   '/home/john doe/Documents'
	 *
	 *   step 1:
	 *     root: 'home'
	 *     entry: 'john doe/Documents'
	 *
	 *   step 2:
	 *     root: 'john doe'
	 *     entry: 'Documents'
	 *
	 * Here, 'Documents' was found at the end of the path,
	 * so 'john doe' is searched for a directory containing
	 * the name 'Documents'.
	 * */

	struct BMFSPath path = *dir_path;

	for (;;)
	{
		struct BMFSPath root;

		int err = bmfs_path_split_root(&path, &root);
		if (err != 0)
		{
			/* path contains the base name, */
			/* enter the directory */
			err = enter_subdir(explorer, &path);
			if (err != 0)
				return err;

			/* done */

			return 0;
		}
		else
		{
			/* root contains a parent directory,
			 * enter the parent directory */
			err = enter_subdir(explorer, &root);
			if (err != 0)
				return err;
		}
	}

	/* unreachable */

	return -EFAULT;
}

