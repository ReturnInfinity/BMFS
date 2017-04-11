/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_DIR_H
#define BMFS_DIR_H

#include "entry.h"

struct BMFSDir
{
	struct BMFSEntry Entries[64];
};

void bmfs_dir_zero(struct BMFSDir *dir);

int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry);

int bmfs_dir_delete(struct BMFSDir *dir, const char *filename);

int bmfs_dir_sort(struct BMFSDir *dir);

struct BMFSEntry * bmfs_dir_find(struct BMFSDir *dir, const char *filename);

#endif /* BMFS_DIR_H */

