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

/* Typedefs */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Global constants */
// Min disk size is 6MiB (three blocks of 2MiB each.)
extern const unsigned int minimumDiskSize;
// Block size is 2MiB
extern const unsigned int blockSize;

/* Global variables */
extern FILE *disk;

/* Entry API */

struct BMFSEntry
{
	char FileName[32];
	u64 StartingBlock;
	u64 ReservedBlocks;
	u64 FileSize;
	u64 Unused;
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
int bmfs_check_tag(FILE *diskfile);
int bmfs_write_tag(FILE *diskfile);
int bmfs_readdir(struct BMFSDir *dir, FILE *diskfile);
int bmfs_writedir(const struct BMFSDir *dir, FILE *diskfile);
int bmfs_findfile(const char *filename, struct BMFSEntry *fileentry, int *entrynumber);
int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel);
unsigned long long bmfs_read(const char *filename,
                             void * buf,
                             unsigned long long len,
                             unsigned long long off);
int bmfs_write(const char *filename,
               const void *buf,
               size_t len,
               off_t off);
void bmfs_readfile(char *filename);
void bmfs_writefile(char *filename);


#endif /* LIBBMFS_H */


/* EOF */
