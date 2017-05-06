/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_DISK_H
#define BMFS_DISK_H

#include "entry.h"
#include "dir.h"

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An abstract disk structure.
 * This structure allows a disk
 * to be represented by anything
 * that can read and write data.
 */

struct BMFSDisk
{
	/** A pointer used by the disk implementation to
	 * pass to the seek, tell, read and write methods.
	 */
	void *disk;
	/** Points the disk to a particular location.
	 */
	int (*seek)(void *disk, int64_t offset, int whence);
	/** Retrieves the current location of the disk.
	 */
	int (*tell)(void *disk, int64_t *offset);
	/** Reads data from the disk.
	 */
	int (*read)(void *disk, void *buf, uint64_t len, uint64_t *read_len);
	/** Writes data to the disk.
	 */
	int (*write)(void *disk, const void *buf, uint64_t len, uint64_t *write_len);
};

int bmfs_disk_init_file(struct BMFSDisk *disk, FILE *file);

int bmfs_disk_seek(struct BMFSDisk *disk, int64_t offset, int whence);
int bmfs_disk_tell(struct BMFSDisk *disk, int64_t *offset);
int bmfs_disk_read(struct BMFSDisk *disk, void *buf, uint64_t len, uint64_t *read_len);
int bmfs_disk_write(struct BMFSDisk *disk, const void *buf, uint64_t len, uint64_t *write_len);

int bmfs_disk_bytes(struct BMFSDisk *disk, size_t *bytes);
int bmfs_disk_mebibytes(struct BMFSDisk *disk, size_t *mebibytes);
int bmfs_disk_blocks(struct BMFSDisk *disk, size_t *blocks);
int bmfs_disk_allocate_bytes(struct BMFSDisk *disk, size_t bytes, size_t *starting_block);
int bmfs_disk_allocate_mebibytes(struct BMFSDisk *disk, size_t mebibytes, size_t *starting_block);
int bmfs_disk_find_file(struct BMFSDisk *disk, const char *filename, struct BMFSEntry *entry, int *number);
int bmfs_disk_read_dir(struct BMFSDisk *disk, struct BMFSDir *dir);
int bmfs_disk_write_dir(struct BMFSDisk *disk, const struct BMFSDir *dir);
int bmfs_disk_check_tag(struct BMFSDisk *disk);
int bmfs_disk_write_tag(struct BMFSDisk *disk);
int bmfs_disk_create_file(struct BMFSDisk *disk, const char *filename, size_t mebibytes);
int bmfs_disk_delete_file(struct BMFSDisk *disk, const char *filename);
int bmfs_disk_format(struct BMFSDisk *disk);

int bmfs_read(struct BMFSDisk *disk,
              const char *filename,
              void * buf,
              uint64_t len,
              uint64_t off);
int bmfs_write(struct BMFSDisk *disk,
               const char *filename,
               const void *buf,
               uint64_t len,
               uint64_t off);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_DISK_H */

