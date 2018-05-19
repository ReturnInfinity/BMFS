/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_CRC32_H
#define BMFS_CRC32_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif

bmfs_uint32 bmfs_crc32(bmfs_uint32 crc, const void *buf, bmfs_uint64 buf_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_CRC32_H */
