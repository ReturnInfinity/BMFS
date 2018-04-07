/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_PATH_H
#define BMFS_PATH_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup path-api Path API
 * Used for manipulating and reading paths.
 * */

/** A path that references a file
 * or directory on the file system.
 * @ingroup path-api
 * */

struct BMFSPath
{
	/** A string pointer containing the characters
	 * of the path. */
	const char *String;
	/** The number of characters in the path, not
	 * including the null terminator. */
	bmfs_uint64 Length;
};

/** Initializes the path to an empty string.
 * @param path The path structure to initialize.
 * @ingroup path-api
 */

void bmfs_path_init(struct BMFSPath *path);

/** Get the base name of the current path.
 * @param path The path to get the base name of.
 * @returns Zero on success, an error code on failure.
 * @ingroup path-api
 * */

int bmfs_path_basename(struct BMFSPath *path);

/** Copies a path from one location to another.
 * @param dst This is the path that will contain the new copy.
 * @param src This is the path that is copied.
 * @ingroup path-api
 * */

void bmfs_path_copy(struct BMFSPath *dst,
                    const struct BMFSPath *src);

/** Inidicates whether or not the path is empty.
 * @param path An initialized path structure.
 * @returns One if the path is empty, zero if
 *  it is not.
 *  @ingroup path-api
 * */

int bmfs_path_empty(const struct BMFSPath *path);

/** Get the parent directory of the path.
 * @param path The path to get the parent of.
 * @returns Zero on success.
 * If the path does not contain a parent
 * directory and is just a base name or '/',
 * when @ref BMFS_ENOENT is returned.
 * @ingroup path-api
 * */

int bmfs_path_parent(struct BMFSPath *path);

/** Gets the root directory of the path.
 * If the path is already a root directory,
 * then @p root is an empty path.
 * @param path An initialized path structure.
 * @param root An uninitialized path structure.
 *  This function will set this parameter to the
 *  root directory of @p path.
 * @returns Zero if the root was found, a negative
 *  number if it wasn't.
 *  @ingroup path-api
 */

int bmfs_path_split_root(struct BMFSPath *path,
                         struct BMFSPath *root);

/** Sets the contents of the path.
 * @param path An initialized path structure.
 * @param string The string containing the path.
 *  The characters pointed to by this address may
 *  be modified by subsequent calls.
 * @param length The length of @p string. This
 *  should not include the null-terminator, if
 *  there is one.
 *  @ingroup path-api
 */

void bmfs_path_set(struct BMFSPath *path,
                   const char *string,
                   bmfs_uint64 length);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_PATH_H */

