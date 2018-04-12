/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_ERRNO_H
#define BMFS_ERRNO_H

/** @defgroup errno Error Codes
 * Describes error codes that may be
 * returned from a BMFS function.
 * */

/** An address passed to a function was set
 * to @ref BMFS_NULL.
 * @ingroup errno
 * */

#define BMFS_EFAULT -1

/** A value passed to a function was invalid.
 * @ingroup errno
 * */

#define BMFS_EINVAL -2

/** The entry does not exist.
 * This could either be a file
 * or a directory.
 * @ingroup errno
 * */

#define BMFS_ENOENT -3

/** A file was requested to be
 * opened, but the path given points
 * to a directory.
 * @ingroup errno
 * */

#define BMFS_EISDIR -4

/** A file or directory was requested
 * to be created at a path that already
 * contains an entry.
 * @ingroup errno
 * */

#define BMFS_EEXIST -5

/** A directory was requested to be opened,
 * but the path pointed to a file instead.
 * @ingroup errno
 * */

#define BMFS_ENOTDIR -6

/** There is no more space left in the disk.
 * @ingroup errno
 * */

#define BMFS_ENOSPC -7

/** This means the function that was called has yet
 * to be implemented.
 * @ingroup errno
 * */

#define BMFS_ENOSYS -8

/** An input/output error occured.
 * @ingroup errno
 * */

#define BMFS_EIO -9

/** Insufficient priviledges to perform
 * the function that was called.
 * @ingroup errno
 * */

#define BMFS_EPERM -10

/** Failed to delete a directory because
 * it still contains entries.
 * @ingroup errno
 * */

#define BMFS_ENOTEMPTY -11

/** Memory allocation failed.
 * @ingroup errno
 * */

#define BMFS_ENOMEM -12

#ifdef __cplusplus
extern "C"
{
#endif

/** Gets a human-readable description
 * of an error code.
 * @param bmfs_errno An error code that was
 * returned by a library function.
 * @returns A null-terminated string
 * that describes the error code.
 * @ingroup errno
 * */

const char *bmfs_strerror(int bmfs_errno);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BMFS_ERRNO_H */
