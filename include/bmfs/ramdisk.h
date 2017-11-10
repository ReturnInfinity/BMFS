/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_RAMDISK_H
#define BMFS_RAMDISK_H

#include "disk.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ramdisk-api Ramdisk API
 * @brief Used for creating file systems in memory.
 * */

/** A BMFS disk formatted in a
 * block of memory. Good for testing
 * and temporary file systems.
 * @ingroup ramdisk-api
 * */

struct BMFSRamdisk
{
	/** The base structure of the disk. */
	struct BMFSDisk base;
	/** The pointer to the start of memory
	 * that the file system resides in. */
	unsigned char *buf;
	/** The number of bytes occupied by the
	 * memory buffer. */
	uint64_t buf_size;
	/** The current position (in bytes) that
	 * the disk is pointing to.
	 * */
	uint64_t buf_pos;
};

/** Initializes the ram disk. This function initializes
 * the structure for pointer-safety. This function must
 * be called before the ramdisk is used.
 * @ingroup ramdisk-api
 * */

void bmfs_ramdisk_init(struct BMFSRamdisk *ramdisk);

/** Called when the ramdisk is no longer going to be used.
 * @ingroup ramdisk-api
 * */

void bmfs_ramdisk_done(struct BMFSRamdisk *ramdisk);

/** Used to set the block of memory that the file
 * system will be created in.
 * @param ramdisk An initialized @ref BMFSRamdisk structure.
 * @param buf The address of the memory block.
 * @param buf_size The number of bytes occupied by the memory
 * block. This must be at least @ref BMFS_MINIMUM_DISK_SIZE.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup ramdisk-api
 * */

int bmfs_ramdisk_set_buf(struct BMFSRamdisk *ramdisk,
                         void *buf, uint64_t buf_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_RAMDISK_H */

