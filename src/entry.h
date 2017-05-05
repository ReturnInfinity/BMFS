/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_ENTRY_H
#define BMFS_ENTRY_H

#include <stdlib.h>
#include <stdint.h>

struct BMFSEntry
{
	char FileName[32];
	uint64_t StartingBlock;
	uint64_t ReservedBlocks;
	uint64_t FileSize;
	uint64_t Unused;
};

void bmfs_entry_init(struct BMFSEntry *entry);

int bmfs_entry_cmp_filename(const struct BMFSEntry *entry, const char *filename);

int bmfs_entry_get_offset(const struct BMFSEntry *entry, uint64_t *offset);

void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename);

void bmfs_entry_set_file_size(struct BMFSEntry *entry, size_t file_size);

void bmfs_entry_set_starting_block(struct BMFSEntry *entry, size_t starting_block);

void bmfs_entry_set_reserved_blocks(struct BMFSEntry *entry, size_t reserved_blocks);

int bmfs_entry_is_empty(const struct BMFSEntry *entry);

int bmfs_entry_is_terminator(const struct BMFSEntry *entry);

#endif /* BMFS_ENTRY_H */

