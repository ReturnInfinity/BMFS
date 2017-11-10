/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_EXPLORER_H
#define BMFS_EXPLORER_H

#include <bmfs/disk.h>
#include <bmfs/dir.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSPath;

/** @defgroup explorer-api Explorer API
 * @brief 
 * */

/** A BMFS file system explorer.
 * Used for exploring the directories
 * of a BMFS formatted file system.
 * @ingroup explorer-api
 * */

struct BMFSExplorer {
	/** @brief The disk containing the filesystem
	 * to explorer. */
	struct BMFSDisk Disk;
	/** @brief The current directory. When the structure
	 * is initialized for the first time, this is the root
	 * directory of the file system.
	 * */
	struct BMFSDir CurrentDirectory;
};

/** Initializes a BMFS file system explorer.
 * @param explorer An uninitialized file system
 * explorer structure.
 * @ingroup explorer-api
 * */

void bmfs_explorer_init(struct BMFSExplorer *explorer);

/** Reads the contents of the current directory.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @param dir A pointer to a @ref BMFSDir structure to get
 *  the directory contents.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 */

int bmfs_explorer_read(struct BMFSExplorer *explorer,
                       struct BMFSDir *dir);

/** Moves the explorer to another directory.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @param dir_path The path to the directory to
 *  move the explorer to.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 * */

int bmfs_explorer_move(struct BMFSExplorer *explorer,
                       const struct BMFSPath *dir_path);


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_EXPLORER_H */

