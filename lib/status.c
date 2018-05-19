/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/status.h>

void bmfs_status_init(struct BMFSStatus *status)
{
	status->TotalSize = 0;
	status->Reserved = 0;
	status->Used = 0;
	status->Wasted = 0;
	status->FileCount = 0;
	status->DirCount = 0;
}
