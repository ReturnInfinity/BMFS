/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_ENCODING_H
#define BMFS_ENCODING_H

/** @file */

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup encoding-api Encoding API
 * Used for serializing and deserializing
 * binary values in a consistent manner.
 * */

/*** Encode a 64-bit unsigned integer.
 * @param n The number to encode.
 * @param buf The buffer to put the value into.
 * This buffer shuold contain at least eight bytes.
 * @ingroup encoding-api
 * */

void bmfs_encode_uint64(bmfs_uint64 n, void *buf);

/*** Encode a 32-bit unsigned integer.
 * @param n The number to encode.
 * @param buf The buffer to put the value into.
 * This buffer should contain at least four bytes.
 * @ingroup encoding-api
 * */

void bmfs_encode_uint32(bmfs_uint32 n, void *buf);

/** Decodes a 64-bit unsigned integer.
 * @param buf The buffer to get the number from.
 * This buffer should contain at least eight bytes.
 * @returns The number that was decoded.
 * @ingroup encoding-api
 * */

bmfs_uint64 bmfs_decode_uint64(const void *buf);

/** Decodes a 32-bit unsigned integer.
 * @param buf The buffer to get the number from.
 * This buffer should contain at least four bytes.
 * @returns The number that was decoded.
 * @ingroup encoding-api
 * */

bmfs_uint32 bmfs_decode_uint32(const void *buf);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_ENCODING_H */
