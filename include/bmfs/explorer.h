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
	/** The disk containing the filesystem
	 * to explorer. */
	struct BMFSDisk Disk;
	/** The current directory. When the structure
	 * is initialized for the first time, this is the root
	 * directory of the file system.
	 * */
	struct BMFSDir CurrentDirectory;
	/** Used for seeking back to the beginning of the
	 * directory for a read or write operation.
	 * */
	uint64_t CurrentDirectoryOffset;
};

/** Initializes a BMFS file system explorer.
 * After calling this function, the disk callbacks
 * should be set. Once the callbacks are set, then
 * call @ref bmfs_explorer_read and then the explorer
 * will be ready for use.
 * @param explorer An uninitialized file system
 * explorer structure.
 * @ingroup explorer-api
 * */

void bmfs_explorer_init(struct BMFSExplorer *explorer);

/** Creates an entry in the current directory
 * of the explorer.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @param entry The entry to create in the current directory.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 * */

int bmfs_explorer_create(struct BMFSExplorer *explorer,
                         const struct BMFSEntry *entry);

/** Creates a file in the current directory of
 * the explorer. The file will be created with one block
 * reserved.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @param filename The name to give the new file.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 * */

int bmfs_explorer_create_file(struct BMFSExplorer *explorer,
                              const char *filename);

/** Reads the contents of the current directory.
 * This should be called after the disk callbacks have been
 * set appropriately.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 */

int bmfs_explorer_read(struct BMFSExplorer *explorer);

/** Writes the contents of the directory to the disk.
 * @param explorer An initialized @ref BMFSExplorer structure.
 * @returns On success, zero. On failure, a negative error code.
 * @ingroup explorer-api
 * */

int bmfs_explorer_write(struct BMFSExplorer *explorer);

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

