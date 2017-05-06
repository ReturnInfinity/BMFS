/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_ENTRY_H
#define BMFS_ENTRY_H

#include <stdlib.h>
#include <stdint.h>

/* @file */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** An entry within a BMFS directory.
 * Contains information on a file, such
 * as name, size and more.
 */

struct BMFSEntry
{
	/** The name of the entry. */
	char FileName[32];
	/** The block that the file data
	 * starts at. */
	uint64_t StartingBlock;
	/** The number of blocks that have
	 * been reserved for the file to grow.
	 */
	uint64_t ReservedBlocks;
	/** The number of bytes in the file
	 * that contain valid data.
	 */
	uint64_t FileSize;
	/** Reserved for future use. Do not
	 * read or write from this field.
	 */
	uint64_t Unused;
};

void bmfs_entry_init(struct BMFSEntry *entry);

int bmfs_entry_cmp_by_filename(const struct BMFSEntry *a,
                               const struct BMFSEntry *b);

int bmfs_entry_cmp_by_starting_block(const struct BMFSEntry *a,
                                     const struct BMFSEntry *b);

int bmfs_entry_cmp_filename(const struct BMFSEntry *entry, const char *filename);

int bmfs_entry_cmp_starting_block(const struct BMFSEntry *entry,
                                  uint64_t starting_block);

int bmfs_entry_get_offset(const struct BMFSEntry *entry, uint64_t *offset);

void bmfs_entry_set_file_name(struct BMFSEntry *entry, const char *filename);

void bmfs_entry_set_file_size(struct BMFSEntry *entry, size_t file_size);

void bmfs_entry_set_starting_block(struct BMFSEntry *entry, size_t starting_block);

void bmfs_entry_set_reserved_blocks(struct BMFSEntry *entry, size_t reserved_blocks);

int bmfs_entry_is_empty(const struct BMFSEntry *entry);

int bmfs_entry_is_terminator(const struct BMFSEntry *entry);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_ENTRY_H */

