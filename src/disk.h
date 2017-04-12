/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_DISK_H
#define BMFS_DISK_H

#include "entry.h"
#include "dir.h"

#include <stdio.h>
#include <sys/types.h>

struct BMFSDisk
{
	void *disk;
	int (*seek)(void *disk, int64_t offset, int whence);
	int (*tell)(void *disk, int64_t *offset);
	int (*read)(void *disk, void *buf, uint64_t len, uint64_t *read_len);
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

int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel);
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
void bmfs_readfile(struct BMFSDisk *disk, const char *filename);
void bmfs_writefile(struct BMFSDisk *disk, const char *filename);

#endif /* BMFS_DISK_H */

