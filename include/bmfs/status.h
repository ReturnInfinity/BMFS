/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_STATUS_H
#define BMFS_STATUS_H

/** @file */

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup status-api Status API
 * Used for determining the usage statistics
 * of the file system.
 * */

/** This structure contains information on
 * space usage of the file system.
 * @ingroup status-api
 * */

struct BMFSStatus
{
	/** The total size available for the file system. */
	bmfs_uint64 TotalSize;
	/** The amount of space that is currently reserved.
	 * Not all of this space is used, but it is not available
	 * to be allocated. */
	bmfs_uint64 Reserved;
	/** This is the amount of space that currently contains data. */
	bmfs_uint64 Used;
	/** This is the amount of space that does not contain data but
	 * is reserved. */
	bmfs_uint64 Wasted;
	/** The number of files in the file system. */
	bmfs_uint64 FileCount;
	/** The number of directories in the file system,
	 * including the root directory. */
	bmfs_uint64 DirCount;
};

/** Initializes the file system status structure.
 * This should be called after the structure is
 * declared.
 * @param status The status structure to initialize.
 * @ingroup status-api
 * */

void bmfs_status_init(struct BMFSStatus *status);

#ifdef __cplusplus
} /* extern "C" */

#endif

#endif /* BMFS_STATUS_H */
