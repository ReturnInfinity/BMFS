/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/errno.h>

const char *bmfs_strerror(int bmfs_errno)
{
	switch (bmfs_errno)
	{
	case 0:
		break;
	case BMFS_EFAULT:
		return "Invalid memory reference";
	case BMFS_EINVAL:
		return "Invalid argument";
	case BMFS_ENOENT:
		return "No such file or directory";
	case BMFS_EISDIR:
		return "Entry is a directory";
	case BMFS_EEXIST:
		return "Entry already exists";
	case BMFS_ENOTDIR:
		return "Entry is not a directory";
	case BMFS_ENOSPC:
		return "No disk space available";
	case BMFS_ENOSYS:
		return "Functionality not implemented";
	case BMFS_EIO:
		return "Input/Output error occured";
	case BMFS_EPERM:
		return "Insufficient permissions";
	case BMFS_ENOTEMPTY:
		return "Directory not empty";
	default:
		return "Unknown error occured";
	}

	return "Success";
}
