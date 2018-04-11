/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_HOST_H
#define BMFS_HOST_H

#include <bmfs/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct BMFSHostData;

/** @defgroup host-api Host API
 * Used for defining functions implemented
 * by the host system, used by the library.
 * */

/** This structure is used to define all of
 * the host specific functions.
 * @ingroup host-api
 * */

struct BMFSHost {
	/** Initializes the host implementation data. */
	struct BMFSHostData *(*Init)(void);
	/** Releases the host implementation data. */
	void (*Done)(struct BMFSHostData *host_data);
	/** Allocates a block of memory. */
	void *(*Malloc)(struct BMFSHostData *host_data, bmfs_uint64 size);
	/** Releases memory allocated by the library. */
	void (*Free)(struct BMFSHostData *host_data, void *addr);
	/** Locks a mutex for the file system. */
	int (*Lock)(struct BMFSHostData *host_data);
	/** Unlocks a mutex for the file system. */
	int (*Unlock)(struct BMFSHostData *host_data);
};

/** Initializes host implementation data.
 * @param host An initialized host structure.
 * @returns This function returns whatever the callback
 * function returns. The return value is not interpreted.
 * Therefore, it may return @ref BMFS_NULL while not indicating
 * an error.
 * @ingroup host-api
 * */

struct BMFSHostData *bmfs_host_init(const struct BMFSHost *host);

/** Called when the host is no longer going to
 * be used by the library.
 * @param host An initialized host structure.
 * @param host_data Host implementation data.
 * @ingroup host-api
 * */

void bmfs_host_done(const struct BMFSHost *host,
                    struct BMFSHostData *host_data);

/** Allocates a block of memory.
 * @param host An initialized host structure.
 * @param host_data Host implementation data.
 * @param size The number of bytes to allocate.
 * @returns On success, the address of the new
 * memory block. On failure, @ref BMFS_NULL.
 * @ingroup host-api
 * */

void *bmfs_host_malloc(const struct BMFSHost *host,
                       struct BMFSHostData *host_data,
                       bmfs_uint64 size);

/** Releases memory allocated by the library.
 * @param host An initialized host structure.
 * @param host_data Host implementation data.
 * @param addr The address of the memory to free.
 * @ingroup host-api
 * */

void bmfs_host_free(const struct BMFSHost *host,
                    struct BMFSHostData *host_data,
                    void *addr);

/** Locks the file system mutex.
 * @param host An initialized host structure.
 * @param host_data Host implementation data.
 * @returns Zero on success. On failure, an
 * error code is returned.
 * @ingroup host-api
 * */

int bmfs_host_lock(const struct BMFSHost *host,
                   struct BMFSHostData *host_data);

/** Unlocks the file system mutex.
 * @param host An initialized host structure.
 * @param host_data Host implementation data.
 * @returns Zero on success. On failure, an
 * error code is returned.
 * @ingroup host-api
 * */

int bmfs_host_unlock(const struct BMFSHost *host,
                     struct BMFSHostData *host_data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BMFS_HOST_H */
