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

/* Global constants */
// Min disk size is 6MiB (three blocks of 2MiB each.)
extern const unsigned int minimumDiskSize;
// Block size is 2MiB
extern const unsigned int blockSize;

/* Global variables */
FILE *file, *disk;
unsigned int filesize, disksize, retval;
char tempfilename[32], tempstring[32];
char *filename, *diskname, *command;
extern const char fs_tag[];
struct BMFSEntry entry;
extern void *pentry;
char *BlockMap;
char *FileBlocks;
char Directory[4096];
char DiskInfo[512];

/* Built-in functions */
int bmfs_findfile(char *filename, struct BMFSEntry *fileentry, int *entrynumber);
void bmfs_list();
void bmfs_format();
int bmfs_initialize(char *diskname, char *size, char *mbr, char *boot, char *kernel);
void bmfs_create(char *filename, unsigned long long maxsize);
void bmfs_read(char *filename);
void bmfs_write(char *filename);
void bmfs_delete(char *filename);


#endif /* LIBBMFS_H */


/* EOF */
