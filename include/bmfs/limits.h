/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_LIMITS_H
#define BMFS_LIMITS_H

#define BMFS_FILE_NAME_MAX 32ULL

#define BMFS_BLOCK_SIZE (1024ULL * 1024ULL * 2ULL)

#define BMFS_MINIMUM_DISK_SIZE (BMFS_BLOCK_SIZE * 3ULL)

#endif /* BMFS_LIMITS_H */

