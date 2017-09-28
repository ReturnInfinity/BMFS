/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_PATH_H
#define BMFS_PATH_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSPath
{
	char *String;
	uint64_t Length;
};

/** Initializes the path to an
 * empty string.
 */

void bmfs_path_init(struct BMFSPath *path);

/** Inidicates whether or not the path is
 * empty.
 * @param path An initialized path structure.
 * @returns One if the path is empty, zero if
 *  it is not.
 * */

int bmfs_path_empty(const struct BMFSPath *path);

/** Gets the root directory of the path.
 * If the path is already a root directory,
 * then @p root is an empty path.
 * @param path An initialized path structure.
 * @param root An uninitialized path structure.
 *  This function will set this parameter to the
 *  root directory of @p path.
 * @returns Zero if the root was found, a negative
 *  number if it wasn't.
 */

int bmfs_path_root(struct BMFSPath *path,
                   struct BMFSPath *root);

/** Sets the contents of the path.
 * @param path An initialized path structure.
 * @param string The string containing the path.
 *  The characters pointed to by this address may
 *  be modified by subsequent calls.
 * @param length The length of @p string. This
 *  should not include the null-terminator, if
 *  there is one.
 */

void bmfs_path_set(struct BMFSPath *path,
                   char *string,
                   uint64_t length);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_PATH_H */

