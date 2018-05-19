/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_STDHOST_H
#define BMFS_STDHOST_H

#include <bmfs/host.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** A host implementation that uses
 * the standard C library.
 * */

extern const struct BMFSHost bmfs_stdhost;

#ifdef __cplusplus
} /* extern "C" */

#endif

#endif /* BMFS_STDHOST_H */
