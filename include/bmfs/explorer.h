/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_EXPLORER_H
#define BMFS_EXPLORER_H

#include <bmfs/disk.h>
#include <bmfs/dir.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSExplorer {
	struct BMFSDisk disk;
	struct BMFSDir dir;
};

void bmfs_explorer_init(struct BMFSExplorer *explorer);

int bmfs_explorer_read_dir(struct BMFSExplorer *explorer,
                           struct BMFSDir *dir);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_EXPLORER_H */

