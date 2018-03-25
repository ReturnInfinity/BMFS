/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_STDLIB_H
#define BMFS_STDLIB_H

#include "bmfs.h"

#include <stdio.h>

/** @file */

#ifdef __cplusplus
extern "C" {
#endif

/** Initializes a disk structure
 * with a FILE structure and the
 * seek, tell, read and write
 * methods from the standard library.
 * @param disk The disk to initialize.
 * @param file A file representing the
 *  disk data.
 * @returns On success, this function
 *  returns zero. If @p disk or @p file
 *  are NULL, this function returns -EFAULT.
 */

int bmfs_disk_init_file(struct BMFSDisk *disk, FILE *file);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_STDLIB_H */

