/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_DIR_H
#define BMFS_DIR_H

#include "entry.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSDisk;

/** @defgroup dir-api Directory API
 * Directory traversal, entry addition
 * or deletion and sorting.
 */

/** A BMFS directory.
 * @ingroup dir-api
 */

struct BMFSDir
{
	/** The disk that contains the directory. */
	struct BMFSDisk *Disk;
	/** The entry containing the position and
	 * other information regarding the directory. */
	struct BMFSEntry Entry;
	/** The entry that was last read from the
	 * client. */
	struct BMFSEntry CurrentEntry;
	/** The index of the current entry that the
	 * next read operation will read. */
	uint64_t CurrentIndex;
};

/** Initializes a directory.
 * @param dir The directory to initialize.
 * @ingroup dir-api
 * */

void bmfs_dir_init(struct BMFSDir *dir);

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

