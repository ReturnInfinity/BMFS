/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_DIR_H
#define BMFS_DIR_H

#include "entry.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSDisk;

/** @defgroup dir-api Directory API
 * Used for reading directory entries.
 */

/** This is a directory structure.
 * It is used for examining the contents
 * of a directory.
 * @ingroup dir-api
 */

struct BMFSDir
{
	/** The disk that contains the directory. */
	struct BMFSDisk *Disk;
	/** The entry containing the position and
	 * other information regarding the directory. */
	struct BMFSEntry Entry;
	/** The offset to the directory entry. This
	 * is used if an entry is added to this directory,
	 * and the directory size must be updated. */
	bmfs_uint64 EntryOffset;
	/** The entry that was last read from the
	 * client. */
	struct BMFSEntry CurrentEntry;
	/** The index of the current entry that the
	 * next read operation will read. */
	bmfs_uint64 CurrentIndex;
	/** Indicates whether or not deleted directories
	 * should be returned from @ref bmfs_dir_next. The
	 * default is that they should not be returned. */
	bmfs_bool IgnoreDeleted;
};

/** Initializes a directory.
 * After calling this function, also
 * set the disk by calling @ref bmfs_dir_set_disk.
 * @param dir The directory to initialize.
 * @ingroup dir-api
 * */

void bmfs_dir_init(struct BMFSDir *dir);

/** Indicate that it is okay to return
 * deleted entries in the directory.
 * @param dir An initialized directory structure.
 * @ingroup dir-api
 * */

void bmfs_dir_view_deleted(struct BMFSDir *dir);

/** Assigns the disk that will be used
 * for the directory operations.
 * @param dir An initialized directory structure.
 * @param disk The disk to assign the directory.
 * @ingroup dir-api
 * */

void bmfs_dir_set_disk(struct BMFSDir *dir,
                       struct BMFSDisk *disk);

/** Imports the directory from the disk.
 * The disk must be currently pointing to
 * the directory entry. If the entry is not
 * a directory, or the read operation fails,
 * then this function will fail. The directory
 * should already have a directory assigned to
 * it (if not, use @ref bmfs_dir_set_disk.)
 * @param dir An initialized directory structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup dir-api
 * */

int bmfs_dir_import(struct BMFSDir *dir);

/** Moves to the next entry in the directory.
 * @param dir An initialized directory.
 * @returns The next entry in the directory.
 * If the end of the directory is reached, then
 * a null pointer is returned.
 * */

const struct BMFSEntry *bmfs_dir_next(struct BMFSDir *dir);

 #ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_DIR_H */

