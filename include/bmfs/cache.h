/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_CACHE_H
#define BMFS_CACHE_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup bmfs-cache Caching API
 * Used for speeding up access to the disk.
 * */

/** Passed to the file system to help
 * increase read and write performance.
 * @ingroup cache-api
 * */

struct BMFSCache {
	/** The start address of the cache. */
	void *addr;
	/** The number of bytes in the cache. */
	bmfs_uint64 size;
};

/** Initializes a cache structure.
 * @param cache The cache structure to initialize.
 * */

void bmfs_cache_init(struct BMFSCache *cache);

/** Sets the starting address of the cache structure.
 * @param cache An initialized cache structure.
 * @param addr The starting address of the cache.
 * @ingroup cache-api
 * */

void bmfs_cache_set_addr(struct BMFSCache *cache, void *addr);

/** Sets the cache size, in bytes.
 * @param cache An initialized cache structure.
 * @param size The size, in bytes, of the cache.
 * @ingroup cache-api
 * */

void bmfs_cache_set_size(struct BMFSCache *cache, bmfs_uint64 size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_CACHE_H */
