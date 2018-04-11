/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/host.h>

#include <bmfs/errno.h>

struct BMFSHostData *bmfs_host_init(const struct BMFSHost *host)
{
	if (host->Init == BMFS_NULL)
		return BMFS_NULL;
	else
		return host->Init();
}

void bmfs_host_done(const struct BMFSHost *host,
                    struct BMFSHostData *host_data)
{
	if (host->Done != BMFS_NULL)
		return host->Done(host_data);
}

void *bmfs_host_malloc(const struct BMFSHost *host,
                       struct BMFSHostData *host_data,
                       bmfs_uint64 size)
{
	if (host->Malloc == BMFS_NULL)
		return BMFS_NULL;
	else
		return host->Malloc(host_data, size);
}

void bmfs_host_free(const struct BMFSHost *host,
                    struct BMFSHostData *host_data,
                    void *addr)
{
	if (host->Free != BMFS_NULL)
		host->Free(host_data, addr);
}

int bmfs_host_lock(const struct BMFSHost *host,
                   struct BMFSHostData *host_data)
{
	if (host->Lock == BMFS_NULL)
		return BMFS_ENOSYS;
	else
		return host->Lock(host_data);
}

int bmfs_host_unlock(const struct BMFSHost *host,
                     struct BMFSHostData *host_data)
{
	if (host->Unlock == BMFS_NULL)
		return BMFS_ENOSYS;
	else
		return host->Unlock(host_data);
}
