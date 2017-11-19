/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

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

/** Initializes a disk with a bootloader, Pure64
 * and a kernel.
 * @param diskname The path to the disk file.
 * @param size The size to truncate the disk
 *  file to. May use one of the suffixes: K,
 *  M, G, T or P.
 * @param mbr Path to bootloader.
 * @param boot Path to Pure64
 * @param kernel Path to kernel.
 * @returns Zero on success.
 */

int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel, char *loader);

/** Reads a file from the disk to a file on
 * the host file system. The file on disk
 * must exist.
 * @param disk The disk to read the file from.
 * @param destination The name of the file on
 * the host filesystem.
 * @param source The name of the file on the
 * BMFS filesystem. If this file is null, then
 * the filename on the BMFS filesystem is the same
 * as teh filename on the host filesystem.
 */

void bmfs_readfile(struct BMFSDisk *disk, const char *destination, const char *source);

/** Writes a file from the host file system
 * to the disk. The disk and the filename
 * on the host file system must be the same.
 * The file must already exist on the disk.
 * @param disk The disk to write the file to.
 * @param destination The name of the file on
 * the BMFS filesystem.
 * @param source The name of the file on the
 * host filesystem. This parameter may be null,
 * in which case the filename of the source is
 * the same as the destination.
 */

void bmfs_writefile(struct BMFSDisk *disk, const char *destination, const char *source);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_STDLIB_H */

