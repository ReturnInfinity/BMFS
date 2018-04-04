/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_SIZE_H
#define BMFS_SIZE_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup size-api Size Specification API
 * Read and write values like 'MiB', 'GB' and others.
 */

/** Specifies which suffix is used
 * in the storage specification.
 * @ingroup size-api
 */

enum BMFSSizeSuffix
{
	/** equivalent to 1024 ^ 0 */
	BMFS_SIZE_SUFFIX_NONE,
	/** equivalent to 1024 ^ 4 */
	BMFS_SIZE_SUFFIX_TEBI,
	/** equivalent to 1024 ^ 3 */
	BMFS_SIZE_SUFFIX_GIBI,
	/** equivalent to 1024 ^ 2 */
	BMFS_SIZE_SUFFIX_MEBI,
	/** equivalent to 1024 */
	BMFS_SIZE_SUFFIX_KIBI,
	/** equivalent to 1000 ^ 4 */
	BMFS_SIZE_SUFFIX_TERA,
	/** equivalent to 1000 ^ 3 */
	BMFS_SIZE_SUFFIX_GIGA,
	/** equivalent to 1000 ^ 2 */
	BMFS_SIZE_SUFFIX_MEGA,
	/** equivalent to 1000 */
	BMFS_SIZE_SUFFIX_KILO
};

/** Used to specify a numerical value of storage capacity.
 * @ingroup size-api
 */

struct BMFSSize
{
	/** The type of storage specification */
	enum BMFSSizeSuffix Suffix;
	/** The number of storage units of the
	 * specifications type. For example: a
	 * value of two and a type of Mebibytes,
	 * would be equivalent to 2MiB. */
	bmfs_uint64 Value;
};

/** Parses a string describing a storage capacity.
 * A couple of examples of strings would be:
 *  - 3KB
 *  - 9B
 *  - 7 (equivalent to 7B)
 *  - 0
 * @param size A storage specification structure.
 *  This parameter is initialized by the function.
 * @param arg A string representation of a storage
 *  capacity. The string must contain no spaces and
 *  is case-sensitive.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup size-api
 */

int bmfs_size_parse(struct BMFSSize *size, const char *arg);

/** Converts a storage specification structure
 * to a human-reabable string.
 * @param size An initialized storage specification.
 * @param str A string to write the storage value to.
 * @param str_len The number of bytes available for @p str.
 *  This value must be at least eight.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup size-api
 */

int bmfs_size_to_string(const struct BMFSSize *size, char *str, bmfs_uint64 str_len);

/** Sets the value, in bytes, of the storage specification.
 * @param size A storage specifier structure. This parameter
 *  is set by the function.
 * @param bytes The value to set the storage specifier to.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup size-api
 */

int bmfs_size_set_bytes(struct BMFSSize *size, bmfs_uint64 bytes);

/** Retrives the number of bytes of the storage specification.
 * This function may fail if the storage specification is not
 * able to be represented by a 64-bit unsigned integer.
 * @param size An initialized storage specification.
 * @param bytes The address of the variable that will receive
 *  the number of bytes of the storage specifier.
 * @returns Zero on success, a negative error on failure.
 * @ingroup size-api
 */

int bmfs_size_bytes(const struct BMFSSize *size, bmfs_uint64 *bytes);

/** Retrives the number of mebibytes of the storage specification.
 * This function may fail if the storage specification is not
 * able to be represented by a 64-bit unsigned integer.
 * @param size An initialized storage specification.
 * @param mebibytes The address of the variable that will receive
 *  the number of mebibytes of the storage specifier.
 * @returns Zero on success, a negative error on failure.
 * @ingroup size-api
 */

int bmfs_size_mebibytes(const struct BMFSSize *size, bmfs_uint64 *mebibytes);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* BMFS_SIZE_H */

