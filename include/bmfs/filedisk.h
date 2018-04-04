/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_FILEDISK_H
#define BMFS_FILEDISK_H

#include <bmfs/disk.h>

#include <stdio.h>

/** @file */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup filedisk-api File Disk API
 * Used for binding a disk to a file.
 * */

/** This structure is made for making it
 * easy to bind a file to a disk structure.
 * It can be used to open and close the file
 * as well.
 * @ingroup filedisk-api
 * */

struct BMFSFileDisk
{
	/** The file containing the disk data. */
	FILE *file;
	/** The structure containing the disk callbacks. */
	struct BMFSDisk base;
};

/** Initializes a file disk for use.
 * @param filedisk The file disk to initialize.
 * @ingroup filedisk-api
 * */

void bmfs_filedisk_init(struct BMFSFileDisk *filedisk);

/** Releases memory allocated by the file disk
 * and closes the file.
 * @param filedisk The file disk to close.
 * @ingroup filedisk-api
 * */

void bmfs_filedisk_done(struct BMFSFileDisk *filedisk);

/** Opens a file for the file disk.
 * If a file is already open, it is closed before
 * the new one is opened.
 * @param filedisk An initialized file disk structure.
 * @param path The path of the file to open.
 * @param mode The mode to open the file in.
 * @returns Zero on success, an error code on failure.
 * @ingroup filedisk-api
 * */

int bmfs_filedisk_open(struct BMFSFileDisk *filedisk,
                       const char *path,
                       const char *mode);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_FILEDISK_H */

