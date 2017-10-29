/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_DIR_H
#define BMFS_DIR_H

#include "entry.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup dir-api Directory API
 * Directory traversal, entry addition
 * or deletion and sorting.
 */

/** A BMFS directory.
 * Contains up to sixty-four entries.
 * @ingroup dir-api
 */

struct BMFSDir
{
	/** The entries array */
	struct BMFSEntry Entries[64];
};

/** Initializes the directory by going
 * through and initializing all sixty
 * four entries.
 * @param dir An uninitialized directory.
 * @ingroup dir-api
 */

void bmfs_dir_init(struct BMFSDir *dir);

/** Adds an entry to the directory.
 *
 * This function fails if the entry already exists.
 *
 * @param dir An initialized directory.
 * @param entry The entry to add to the directory.
 * @returns Zero on success, a negative error code
 *  on failure.
 * @ingroup dir-api
 */

int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry);

/** Adds a file, by filename, to the directory.
 * This function does what @ref bmfs_dir_add does,
 * but it doesn't not require the creation of an
 * entry structure by the caller.
 * @param dir An initialized directory.
 * @param filename An ASCII, null-terminated filename.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup dir-api
 */

int bmfs_dir_add_file(struct BMFSDir *dir, const char *filename);

/** Creates a new sub-directory. This function allows
 * the creation of a directory without creating and
 * initialized a directory structure.
 * @param dir The directory to add the sub-directory to.
 * @param dirname The name of the new directory.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup dir-api
 */

int bmfs_dir_add_subdir(struct BMFSDir *dir, const char *dirname);

/** Removes an entry from the directory.
 * If the file doesn't exist, this function fails.
 * @param dir An initialized directory.
 * @param filename An ASCII, null-terminated filename.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup dir-api
 */

int bmfs_dir_delete_file(struct BMFSDir *dir, const char *filename);

/** Sorts the directory.
 * @param dir An initialized directory.
 * @param entry_cmp The function used to
 *  compare two directory entries. If this
 *  parameter is NULL, then the directory
 *  is sorted alphabetically.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup dir-api
 */

int bmfs_dir_sort(struct BMFSDir *dir, int (*entry_cmp)(const struct BMFSEntry *a, const struct BMFSEntry *b));

/** Locates an entry by filename.
 * @param dir An initialized directory.
 * @param filename The filename to search
 *  for.
 * @returns If the entry is found, a pointer
 *  to the structure is returned. If it is not
 *  found, NULL is returned instead.
 * @ingroup dir-api
 */

struct BMFSEntry * bmfs_dir_find(struct BMFSDir *dir, const char *filename);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_DIR_H */

