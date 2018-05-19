/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_TYPES_H
#define BMFS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup types Type Definitions
 * Used for writing common types.
 * */

/** The largest unsigned type on the system.
 * @ingroup types
 * */

typedef unsigned long long int bmfs_size;

/** An unsigned integer, at least 64-bits in size.
 * @ingroup types
 * */

typedef unsigned long long int bmfs_uint64;

/** An unsigned integer, at least 32-bits in size.
 * @ingroup types
 * */

typedef unsigned int bmfs_uint32;

/** An unsigned integer, at least 16-bits in size.
 * @ingroup types
 * */

typedef unsigned short int bmfs_uint16;

/** An unsigned integer, exactly 8-bits in size.
 * @ingroup types
 * */

typedef unsigned char bmfs_uint8;

/** A boolean type.
 * Use @ref BMFS_TRUE and @ref BMFS_FALSE
 * to assign to and read from this type.
 * @ingroup types
 * */

typedef unsigned char bmfs_bool;

/** Represents a true value in a boolean type.
 * @ingroup types
 * */

#define BMFS_TRUE 1

/** Represents a false value in a boolean type.
 * @ingroup types
 * */

#define BMFS_FALSE 0

/** Used to indicate a pointer that does not
 * point to anything.
 * @ingroup types
 * */

#define BMFS_NULL ((void *) 0)

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TYPES_H */
