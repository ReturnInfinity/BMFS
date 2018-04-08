/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_HEADER_H
#define BMFS_HEADER_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup header-api Header API
 * Used for reading and writing the
 * file system header.
 * */

struct BMFSDisk;

/** Describes various positions and
 * characteristics of the file system.
 * It is the first structure in the
 * layout.
 * @ingroup header-api
 * */

struct BMFSHeader
{
	/** The BMFS file system tag.
	 * If this is not set to 'BMFS',
	 * then the file system is not
	 * BMFS formatted. */
	char Signature[8];
	/** The offset, in bytes, of the root directory.*/
	bmfs_uint64 RootOffset;
	/** The offset, in bytes, of the allocation table. */
	bmfs_uint64 TableOffset;
	/** The number of used entries in the allocation table. */
	bmfs_uint64 TableEntryCount;
	/** The total size allowed for the file system to grow. */
	bmfs_uint64 TotalSize;
	/** The number of blocks per byte. */
	bmfs_uint64 BlockSize;
	/** Padding until the 512-byte marker.
	 * This space is alos reserved for future use. */
	bmfs_uint64 Reserved[58];
};

/** Initializes a file system header.
 * @param header The header to initialize.
 * @ingroup header-api
 * */

void bmfs_header_init(struct BMFSHeader *header);

/** Checks to make sure that a header is valid.
 * This is useful after reading a header from disk.
 * @param header An initialized header structure.
 * @returns Zero if the header is valid, an error code
 * if it is not.
 * @ingroup header-api
 * */

int bmfs_header_check(const struct BMFSHeader *header);

/** Reads a file system header from a disk.
 * @param header An initialized header structure.
 * @param disk The disk to read the header from.
 * @returns Zero on success, an error code on failure.
 * @ingroup header-api
 * */

int bmfs_header_read(struct BMFSHeader *header,
                     struct BMFSDisk *disk);

/** Writes a header to disk.
 * @param header The header to write to disk.
 * @param disk The disk to write the header to.
 * @returns Zero on success, an error code on failure.
 * @ingroup header-api
 * */

int bmfs_header_write(const struct BMFSHeader *header,
                      struct BMFSDisk *disk);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_HEADER_H */

