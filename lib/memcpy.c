/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include "memcpy.h"

void bmfs_memcpy(void *dst, const void *src, bmfs_uint64 size)
{
	unsigned char *dst8 = (unsigned char *) dst;
	const unsigned char *src8 = (const unsigned char *) src;

	for (bmfs_uint64 i = 0; i < size; i++)
		dst8[i] = src8[i];
}
