/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/stdhost.h>

#include <bmfs/errno.h>

#include <stdlib.h>

struct BMFSHostData
{
	/* Nothing required here yet. */
};

struct BMFSHostData *stdhost_init(void)
{
	struct BMFSHostData *host_data = malloc(sizeof(*host_data));
	if (host_data == NULL)
		return BMFS_NULL;

	return host_data;
}

void stdhost_done(struct BMFSHostData *host_data)
{
	if (host_data == BMFS_NULL)
		return;

	free(host_data);
}

void *stdhost_malloc(struct BMFSHostData *host_data,
                     bmfs_uint64 size)
{
	if (host_data == BMFS_NULL)
		return BMFS_NULL;

	void *addr = malloc(size);
	if (addr == NULL)
		return BMFS_NULL;

	return addr;
}

void stdhost_free(struct BMFSHostData *host_data,
                  void *addr)
{
	if (host_data == BMFS_NULL)
		return;

	free(addr);
}

int stdhost_lock(struct BMFSHostData *host_data)
{
	if (host_data == BMFS_NULL)
		return BMFS_EFAULT;

	return BMFS_ENOSYS;
}

int stdhost_unlock(struct BMFSHostData *host_data)
{
	if (host_data == BMFS_NULL)
		return BMFS_EFAULT;

	return BMFS_ENOSYS;
}

const struct BMFSHost bmfs_stdhost = {
	stdhost_init,
	stdhost_done,
	stdhost_malloc,
	stdhost_free,
	stdhost_lock,
	stdhost_unlock
};
