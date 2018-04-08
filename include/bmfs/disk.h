/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_DISK_H
#define BMFS_DISK_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup disk-api Disk API
 * Read and write from a formatted disk.
 */

/** Used with the function @ref bmfs_disk_seek
 * to indicate that the reference point of the
 * seek operation is at the beginning of the disk.
 * @ingroup disk-api
 * */

#define BMFS_SEEK_SET 0

/** Used with the function @ref bmfs_disk_seek
 * to indicate that the reference point of the
 * seek operation is at the end of the disk.
 * @ingroup disk-api
 * */

#define BMFS_SEEK_END 2

/** An abstract disk structure.
 * This structure allows a disk
 * to be represented by anything
 * that can read and write data.
 * @ingroup disk-api
 */

struct BMFSDisk
{
	/** A pointer used by the disk implementation to
	 * pass to the seek, tell, read and write methods. */
	void *disk;
	/** Called when the disk is no longer going to be
	 * used. */
	void (*done)(void *disk);
	/** Points the disk to a particular location. */
	int (*seek)(void *disk, bmfs_uint64 offset, int whence);
	/** Retrieves the current location of the disk. */
	int (*tell)(void *disk, bmfs_uint64 *offset);
	/** Reads data from the disk. */
	int (*read)(void *disk, void *buf, bmfs_uint64 len, bmfs_uint64 *read_len);
	/** Writes data to the disk. */
	int (*write)(void *disk, const void *buf, bmfs_uint64 len, bmfs_uint64 *write_len);
};

/** Initializes the members of the disk structure
 * for pointer safety. This function must be called
 * before any others.
 * @param disk The disk to initialize.
 * @ingroup disk-api
 * */

void bmfs_disk_init(struct BMFSDisk *disk);

/** Called when the disk is no longer going
 * to be used by the caller.
 * @param disk An initialized disk structure.
 * @ingroup disk-api
 * */

void bmfs_disk_done(struct BMFSDisk *disk);

/** Points the disk to a particular offset.
 * @param disk An initialized disk.
 * @param offset The offset to point the disk to.
 * @param whence Where the base the offset from.
 *  This value may be @ref BMFS_SEEK_SET or @ref BMFS_SEEK_END.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_seek(struct BMFSDisk *disk,
                   bmfs_uint64 offset,
                   int whence);

/** Retrieves the current offset of the disk.
 * @param disk An initialized disk.
 * @param offset A pointer to the variable that
 *  will receive the disk offset.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_tell(struct BMFSDisk *disk,
                   bmfs_uint64 *offset);

/** Reads data from the disk.
 * @param disk An initialized disk.
 * @param buf Where to put the data
 *  read from the disk.
 * @param len The number of bytes
 *  available in @p buf.
 * @param read_len A pointer to the
 *  variable that will receive the
 *  number of bytes read from the
 *  disk. This field may be NULL.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_read(struct BMFSDisk *disk,
                   void *buf,
                   bmfs_uint64 len,
                   bmfs_uint64 *read_len);

/** Writes data to the disk.
 * @param disk An initialized disk.
 * @param buf Contains the data to
 *  be written to disk.
 * @param len The number of bytes in
 *  @p buf to write to the disk.
 * @param write_len A pointer to the
 *  variable that will receive the
 *  number of bytes written to disk.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_write(struct BMFSDisk *disk,
                    const void *buf,
                    bmfs_uint64 len,
                    bmfs_uint64 *write_len);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_DISK_H */
