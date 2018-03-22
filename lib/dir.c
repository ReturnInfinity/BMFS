/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#include <bmfs/dir.h>
#include <errno.h>

static int sort_entries(struct BMFSEntry *a,
                        struct BMFSEntry *b,
                        int (*entry_cmp)(const struct BMFSEntry *a,
                                         const struct BMFSEntry *b));

void bmfs_dir_init(struct BMFSDir *dir)
{
	for (uint64_t i = 0; i < 64; i++)
	{
		bmfs_entry_init(&dir->Entries[i]);
	}
}

int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry)
{
	if (bmfs_dir_find(dir, entry->Name) != NULL)
		return -EEXIST;

	for (size_t i = 0; i < 64; i++)
	{
		struct BMFSEntry *dst;
		dst = &dir->Entries[i];
		if (bmfs_entry_is_empty(dst))
		{
			*dst = *entry;
			return 0;
		}
		else if (bmfs_entry_is_terminator(dst))
		{
			*dst = *entry;
			if ((i + 1) < 64)
				/* make sure next entry
				 * indicates end of directory */
				dir->Entries[i + 1].Name[0] = 0;
			return 0;
		}
	}
	/* ran out of entries */
	return -ENOSPC;
}

int bmfs_dir_add_file(struct BMFSDir *dir, const char *filename)
{
	if ((dir == NULL)
	 || (filename == NULL))
		return -EFAULT;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_file_name(&entry, filename);
	bmfs_entry_set_starting_block(&entry, 1);
	bmfs_entry_set_type(&entry, BMFS_TYPE_FILE);
	return bmfs_dir_add(dir, &entry);
}

int bmfs_dir_add_subdir(struct BMFSDir *dir, const char *dirname)
{
	if ((dir == NULL)
	 || (dirname == NULL))
		return -EFAULT;

	struct BMFSEntry entry;
	bmfs_entry_init(&entry);
	bmfs_entry_set_file_name(&entry, dirname);
	bmfs_entry_set_starting_block(&entry, 1);
	bmfs_entry_set_type(&entry, BMFS_TYPE_DIRECTORY);
	return bmfs_dir_add(dir, &entry);
}

int bmfs_dir_delete_file(struct BMFSDir *dir, const char *filename)
{
	struct BMFSEntry *entry;

	entry = bmfs_dir_find(dir, filename);
	if (entry == NULL)
		return -ENOENT;

	entry->Name[0] = 1;

	return 0;
}

int bmfs_dir_sort(struct BMFSDir *dir, int (*entry_cmp)(const struct BMFSEntry *a, const struct BMFSEntry *b))
{
	if (entry_cmp == NULL)
		entry_cmp = bmfs_entry_cmp_by_filename;

	size_t i;
	struct BMFSEntry *a;
	struct BMFSEntry *b;
	int mod_flag = 0;

	do {
		mod_flag = 0;
		for (i = 0; i < 64 - 1; i++)
		{
			a = &dir->Entries[i];
			if (bmfs_entry_is_terminator(a))
				break;

			b = &dir->Entries[i + 1];
			if (bmfs_entry_is_terminator(b))
				break;

			mod_flag |= sort_entries(a, b, entry_cmp);
		}
	} while (mod_flag);

	return 0;
}

struct BMFSEntry * bmfs_dir_find(struct BMFSDir *dir, const char *filename)
{
	int tint;
	struct BMFSEntry *entry;

	for (tint = 0; tint < 64; tint++)
	{
		entry = &dir->Entries[tint];
		if (bmfs_entry_is_terminator(entry))
			/* end of directory */
			break;
		else if (bmfs_entry_is_empty(entry))
			/* skip empty entry */
			continue;
		else if (bmfs_entry_cmp_filename(&dir->Entries[tint], filename) != 0)
			/* not a match, skip this file */
			continue;

		/* file was found */
		return entry;
	}

	/* file not found */
	return NULL;
}

static int sort_entries(struct BMFSEntry *a,
                        struct BMFSEntry *b,
                        int (*entry_cmp)(const struct BMFSEntry *a,
                                         const struct BMFSEntry *b))
{
	int res;
	struct BMFSEntry tmp;
	res = entry_cmp(a, b);
	if (res > 0)
	{
		tmp = *a;
		*a = *b;
		*b = tmp;
		return 1;
	}
	return 0;
}

