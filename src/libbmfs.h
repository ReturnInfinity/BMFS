/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef LIBBMFS_H
#define LIBBMFS_H

/* Global includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>

/* Global constants */
// Min disk size is 6MiB (three blocks of 2MiB each.)
extern const unsigned int minimumDiskSize;
// Block size is 2MiB
extern const unsigned int blockSize;

/* Entry API */

struct BMFSEntry
{
	char FileName[32];
	uint64_t StartingBlock;
	uint64_t ReservedBlocks;
	uint64_t FileSize;
	uint64_t Unused;
};

void bmfs_entry_zero(struct BMFSEntry *entry);
void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename);
void bmfs_entry_set_file_size(struct BMFSEntry *entry, size_t file_size);
void bmfs_entry_set_starting_block(struct BMFSEntry *entry, size_t starting_block);
void bmfs_entry_set_reserved_blocks(struct BMFSEntry *entry, size_t reserved_blocks);
int bmfs_entry_is_empty(const struct BMFSEntry *entry);
int bmfs_entry_is_terminator(const struct BMFSEntry *entry);

/* Directory API */

struct BMFSDir
{
	struct BMFSEntry Entries[64];
};

void bmfs_dir_zero(struct BMFSDir *dir);
int bmfs_dir_add(struct BMFSDir *dir, const struct BMFSEntry *entry);
int bmfs_dir_delete(struct BMFSDir *dir, const char *filename);
int bmfs_sortdir(struct BMFSDir *dir);
struct BMFSEntry * bmfs_find(struct BMFSDir *dir, const char *filename);

/* Disk API */

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

int bmfs_disk_find_file(struct BMFSDisk *disk, const char *filename, struct BMFSEntry *entry, int *number);
int bmfs_disk_read_dir(struct BMFSDisk *disk, struct BMFSDir *dir);
int bmfs_disk_write_dir(struct BMFSDisk *disk, const struct BMFSDir *dir);

int bmfs_disk_allocate_bytes(FILE *diskfile, size_t bytes, size_t *starting_block);
int bmfs_disk_allocate_mebibytes(FILE *diskfile, size_t mebibytes, size_t *starting_block);
int bmfs_disk_bytes(FILE *diskfile, size_t *bytes);
int bmfs_disk_mebibytes(FILE *diskfile, size_t *mebibytes);
int bmfs_disk_blocks(FILE *diskfile, size_t *blocks);
int bmfs_disk_format(FILE *diskfile);
int bmfs_disk_create_file(FILE *diskfile, const char *filename, size_t mebibytes);
int bmfs_disk_delete_file(FILE *diskfile, const char *filename);
int bmfs_disk_set_bytes(FILE *diskfile, size_t bytes);
int bmfs_disk_set_mebibytes(FILE *diskfile, size_t mebibytes);
int bmfs_disk_set_blocks(FILE *diskfile, size_t blocks);
int bmfs_disk_check_tag(FILE *diskfile);
int bmfs_disk_write_tag(FILE *diskfile);
int bmfs_readdir(struct BMFSDir *dir, FILE *diskfile);
int bmfs_writedir(const struct BMFSDir *dir, FILE *diskfile);
int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel);
unsigned long long bmfs_read(FILE *diskfile,
                             const char *filename,
                             void * buf,
                             unsigned long long len,
                             unsigned long long off);
int bmfs_write(FILE *diskfile,
               const char *filename,
               const void *buf,
               size_t len,
               off_t off);
void bmfs_readfile(FILE *diskfile, char *filename);
void bmfs_writefile(FILE *diskfile, char *filename);


#endif /* LIBBMFS_H */


/* EOF */
