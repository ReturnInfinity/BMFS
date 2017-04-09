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

/* Global defines */
struct BMFSEntry
{
	char FileName[32];
	u64 StartingBlock;
	u64 ReservedBlocks;
	u64 FileSize;
	u64 Unused;
};
struct BMFSDir
{
	struct BMFSEntry Entries[64];
};

/* Global constants */
// Min disk size is 6MiB (three blocks of 2MiB each.)
extern const unsigned int minimumDiskSize;
// Block size is 2MiB
extern const unsigned int blockSize;

/* Global variables */
extern FILE *disk;

/* Built-in functions */
int bmfs_disk_bytes(FILE *diskfile, size_t *bytes);
int bmfs_disk_mebibytes(FILE *diskfile, size_t *mebibytes);
int bmfs_disk_blocks(FILE *diskfile, size_t *blocks);
int bmfs_check_tag(FILE *diskfile);
int bmfs_write_tag(FILE *diskfile);
void bmfs_dir_zero(struct BMFSDir *dir);
int bmfs_opendir(struct BMFSDir *dir, const char *path);
int bmfs_readdir(struct BMFSDir *dir, FILE *diskfile);
int bmfs_savedir(const struct BMFSDir *dir, const char *path);
int bmfs_writedir(const struct BMFSDir *dir, FILE *diskfile);
struct BMFSEntry * bmfs_find(struct BMFSDir *dir, const char *filename);
int bmfs_findfile(const char *filename, struct BMFSEntry *fileentry, int *entrynumber);
void bmfs_list();
void bmfs_format();
int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel);
int bmfs_create(const char *filename, unsigned long long maxsize);
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
void bmfs_delete(const char *filename);


#endif /* LIBBMFS_H */


/* EOF */
