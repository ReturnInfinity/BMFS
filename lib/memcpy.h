/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_MEMCPY_H
#define BMFS_MEMCPY_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

void bmfs_memcpy(void *dst, const void *src, bmfs_uint64 size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BMFS_MEMCPY_H */
