/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include "dir.h"

#include <errno.h>
#include <string.h>

static int StartingBlockCmp(const void *pa, const void *pb);

void bmfs_dir_zero(struct BMFSDir *dir)
{
	memset(dir, 0, sizeof(*dir));
}

int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry)
{
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
				dir->Entries[i + 1].FileName[0] = 0;
			return 0;
		}
	}
	/* ran out of entries */
	return -ENOSPC;
}

int bmfs_dir_sort(struct BMFSDir *dir)
{
	if (dir == NULL)
		return -EFAULT;
	size_t i;
	for (i = 0; i < 64; i++)
	{
		if (dir->Entries[i].FileName[0] == 0)
			break;
	}
	/* i is the number of used entries */
	qsort(dir->Entries, i, sizeof(dir->Entries[0]), StartingBlockCmp);
	return 0;
}

struct BMFSEntry * bmfs_dir_find(struct BMFSDir *dir, const char *filename)
{
	int tint;
	for (tint = 0; tint < 64; tint++)
	{
		if (dir->Entries[tint].FileName[0] == 0)
			/* end of directory */
			break;
		else if (dir->Entries[tint].FileName[0] == 1)
			/* skip empty entry */
			continue;
		else if (strcmp(dir->Entries[tint].FileName, filename) != 0)
			/* not a match, skip this file */
			continue;

		/* file was found */
		return &dir->Entries[tint];
	}

	/* file not found */
	return NULL;
}

// helper function for qsort, sorts by StartingBlock field
static int StartingBlockCmp(const void *pa, const void *pb)
{
	struct BMFSEntry *ea = (struct BMFSEntry *)pa;
	struct BMFSEntry *eb = (struct BMFSEntry *)pb;
	// empty records go to the end
	if (ea->FileName[0] == 0x01)
		return 1;
	if (eb->FileName[0] == 0x01)
		return -1;
	// compare non-empty records by their starting blocks number
	return (ea->StartingBlock - eb->StartingBlock);
}

