/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_ENTRY_H
#define BMFS_ENTRY_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup entry-api Entries
 * Examine and manipulate file entry data.
 */

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

/** Initializes an entry.
 * @param entry An uninitialized entry.
 * @ingroup entry-api
 */

void bmfs_entry_init(struct BMFSEntry *entry);

/** Compares two entries by file name.
 * This function can be used by @ref bmfs_dir_sort.
 * @param a An initialized entry.
 * @param b An additional initialized entry.
 * @returns If the entries have the same
 *  file name, zero is returned. If entry
 *  @p a contains a byte higher than entry
 *  @p b, then a number greater than zero
 *  is returned. If the opposite occurs, then
 *  a negative number is returned.
 * @ingroup entry-api
 */

int bmfs_entry_cmp_by_filename(const struct BMFSEntry *a,
                               const struct BMFSEntry *b);

/** Compares two entries by starting block.
 * This function can be used by @ref bmfs_dir_sort.
 * @param a An initialized entry.
 * @param b An additional initialized entry.
 * @returns If the entries have the same
 *  starting block, zero is returned. If entry
 *  @p a contains a starting block greater than
 *  entry @p b, then a number greater than zero
 *  is returned. If the opposite occurs, then
 *  a negative number is returned.
 * @ingroup entry-api
 */

int bmfs_entry_cmp_by_starting_block(const struct BMFSEntry *a,
                                     const struct BMFSEntry *b);

/** Compares an entry's file name with
 * another file name parameter.
 * @param entry An initialized entry.
 * @param filename A file name to compare.
 * @returns Zero if the file name in the
 *  entry matches the file name in the
 *  parameter, one if the entry's file
 *  name is greater, negative one if it's
 *  less than the file name parameter.
 * @ingroup entry-api
 */

int bmfs_entry_cmp_filename(const struct BMFSEntry *entry,
                            const char *filename);

/** Compares an entry's starting block with
 * another starting block.
 * @param entry The entry containing a starting
 *  block.
 * @param starting_block The starting block to
 *  compare with.
 * @returns Zero on success, if they're equal.
 *  If the starting block in @p entry is greater
 *  than @p starting_block, then one is returned.
 *  If the starting block in @p entry is less
 *  than @p starting_block, then negative one is
 *  returned.
 * @ingroup entry-api
 */

int bmfs_entry_cmp_starting_block(const struct BMFSEntry *entry,
                                  uint64_t starting_block);

/** Gets the absolute byte offset of the
 * entry on disk.
 * @param entry An initialized entry.
 * @param offset A pointer to a variable
 *  that will receive the byte offset of
 *  the entry.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup entry-api
 */

int bmfs_entry_get_offset(const struct BMFSEntry *entry,
                          uint64_t *offset);

/** Sets the file name of an entry.
 * @param entry An initialized or
 *  uninitialized entry.
 * @param filename The new file name
 *  of the entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_file_name(struct BMFSEntry *entry,
                              const char *filename);

/** Sets the file size of the entry.
 * @param entry An initialized or
 *  uninitialized entry.
 * @param file_size The new file size
 *  of the entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_file_size(struct BMFSEntry *entry,
                              uint64_t file_size);

/** Sets the starting block of the entry.
 * @param entry An initialized or
 *  uninitialized entry.
 * @param starting_block The new starting
 *  block of the entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_starting_block(struct BMFSEntry *entry,
                                   uint64_t starting_block);

/** Sets the number of reserved blocks
 * for the entry.
 * @param entry An initialized or
 *  uninitialized entry.
 * @param reserved_blocks The new number
 *  of reserved blocks for the entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_reserved_blocks(struct BMFSEntry *entry,
                                    uint64_t reserved_blocks);

/** Indicates wether or not the
 * entry is empty.
 * @param entry An initialized entry.
 * @returns One if the entry is the
 *  empty, zero if it is not.
 * @ingroup entry-api
 */

int bmfs_entry_is_empty(const struct BMFSEntry *entry);

/** Indicates wether or not the
 * entry marks the end of the directory.
 * @param entry An initialized entry.
 * @returns One if the entry is the
 *  end-of-directory marker, zero if
 *  it is not.
 * @ingroup entry-api
 */

int bmfs_entry_is_terminator(const struct BMFSEntry *entry);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_ENTRY_H */

