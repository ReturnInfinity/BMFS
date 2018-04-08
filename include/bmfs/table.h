/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_TABLE_H
#define BMFS_TABLE_H

#include <bmfs/types.h>

/** @defgroup table-api Table API
 * Examine and manipulate the disk space
 * allocation table.
 */

/** Indicates the maximum number of entries
 * in the disk space allocation table.
 * @ingroup table-api
 * */

#define BMFS_TABLE_ENTRY_COUNT_MAX 1024

/** This flag goes into a table entry and
 * indicates that the allocation was deleted.
 * The allocation is kept for restoration purposes.
 * @ingroup table-api
 * */

#define BMFS_TABLE_FLAG_DELETED 0x01

/** This flag goes into a table entry and
 * indicates that the allocation is encrypted.
 * @ingroup table-api
 * */

#define BMFS_TABLE_FLAG_ENCRYPTED 0x02

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSDisk;

/** A header in the allocation table,
 * describing an occupied region on
 * the disk. The location of the table
 * entries is indicated by the file system
 * header.
 * @ingroup table-api
 * */

struct BMFSTableEntry
{
	/** The offset, in bytes, of the region. */
	bmfs_uint64 Offset;
	/** The number of bytes reserved for the region. */
	bmfs_uint64 Reserved;
	/** Flags associated with the allocation. */
	bmfs_uint32 Flags;
	/** Checksum of the entry. */
	bmfs_uint32 Checksum;
};

/** Initializes a table entry.
 * @param entry The table entry to initialize.
 * @ingroup table-api
 * */

void bmfs_table_entry_init(struct BMFSTableEntry *entry);

/** Checks the table entry for data integrity.
 * @param entry The entry to check.
 * @returns Zero on success, an error code on failure.
 * If the table entry has been determined to be invalid,
 * then @ref BMFS_EINVAL is returned.
 * @ingroup table-api
 * */

int bmfs_table_entry_check(const struct BMFSTableEntry *entry);

/** Reads a table entry from the current position on the disk.
 * @param entry The structure to put the entry data into.
 * @param disk The disk to read the entry from.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_entry_read(struct BMFSTableEntry *entry,
                          struct BMFSDisk *disk);

/** Writes a table entry to disk at the current location.
 * @param entry The entry to write to disk.
 * @param disk The disk to write the entry to.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_entry_write(const struct BMFSTableEntry *entry,
                           struct BMFSDisk *disk);

/** Indicates whether or not the allocation
 * entry was deleted or not.
 * @param entry An initialized table entry structure.
 * @returns @ref BMFS_TRUE if the entry is deleted
 * and @ref BMFS_FALSE if it was not.
 * @ingroup table-api
 * */

bmfs_bool bmfs_table_entry_is_deleted(const struct BMFSTableEntry *entry);

/** Set the 'deleted' flag bit so that this entry
 * is not counted in other allocation calls.
 * @param entry The entry to mark as deleted.
 * */

void bmfs_table_entry_set_deleted(struct BMFSTableEntry *entry);

/** The disk allocation table.
 * Used for allocation space on the disk
 * and ensuring that no two allocations
 * overlap.
 * @ingroup table-api
 * */

struct BMFSTable
{
	/** A pointer to the associated disk. */
	struct BMFSDisk *Disk;
	/** A placeholder for the current entry. */
	struct BMFSTableEntry CurrentEntry;
	/** The offset of the table on disk. */
	bmfs_uint64 TableOffset;
	/** The number of entries in the table. */
	bmfs_uint64 EntryCount;
	/** The index of the current table entry. */
	bmfs_uint64 EntryIndex;
	/** The maximum offset that an allocation can reach. */
	bmfs_uint64 MaxOffset;
	/** The mininum offset of an allocation. */
	bmfs_uint64 MinOffset;
	/** The block size to use when creating allocations. */
	bmfs_uint64 BlockSize;
	/** If true, ignore entries that are deleted. */
	bmfs_bool IgnoreDeleted;
};

/** Initializes an allocation table.
 * Call @ref bmfs_table_set_disk, @ref bmfs_table_set_offset,
 * and @ref bmfs_table_set_count before calling any other functions.
 * @param table The allocation table to initialize.
 * @ingroup table-api
 * */

void bmfs_table_init(struct BMFSTable *table);

/** When iterating the table, allow for the
 * deleted entries to be returned.
 * @param table An initialized allocation table.
 * @ingroup table-api
 * */

void bmfs_table_view_deleted(struct BMFSTable *table);

/** When iterating the table, do not return
 * the entries that have been deleted. This
 * is the default behavior.
 * @param table An initialized allocation table.
 * @ingroup table-api
 * */

void bmfs_table_hide_deleted(struct BMFSTable *table);

/** Locate a table entry by the offset.
 * This is useful for finding out how
 * much space is reserved for an entry.
 * @param table An initialized table structure.
 * @param offset The offset of the table entry.
 * @returns On success, the table entry containing
 * the same offset. On failure, @ref BMFS_NULL
 * is returned.
 * @ingroup table-api
 * */

struct BMFSTableEntry *bmfs_table_find(struct BMFSTable *table,
                                       bmfs_uint64 offset);

/** Allocates space on the disk, adding an
 * entry to the allocation table.
 * @param table An initialized table structure.
 * @param size The size, in bytes, that the allocation
 * should be able to contain.
 * @param offset A pointer that will receive the offset
 * of the allocation (not the allocation entry, the allocation
 * region on disk.)
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_alloc(struct BMFSTable *table,
                     bmfs_uint64 size,
                     bmfs_uint64 *offset);

/** Re-allocate space to fit a new size, relocating
 * the space if required.
 * @param table An initialized table structure.
 * @param new_size The new size that the allocation
 * should be able to fit.
 * @param offset This parameter is both an input
 * and an output. Initially, it should point to the
 * allocation to resize. Afterwards, it will point
 * to the new allocation (if a new one was made.)
 * @returns Zero on success, an error code on failure.
 * */

int bmfs_table_realloc(struct BMFSTable *table,
                       bmfs_uint64 new_size,
                       bmfs_uint64 *offset);

/** Removes an allocation from the table.
 * @param table An initialized table structure.
 * @param offset The offset of the allocation to free.
 * @returns Zero on success, an error code on failure.
 * */

int bmfs_table_free(struct BMFSTable *table,
                    bmfs_uint64 offset);

/** Import the table from the disk.
 * @param table An initialized table structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_import(struct BMFSTable *table);

/** Goes to the beginning of the table.
 * @param table An initialized table structure.
 * @ingroup table-api
 * */

void bmfs_table_begin(struct BMFSTable *table);

/** Goes to the end of the table.
 * @param table An initialized table structure.
 * @ingroup table-api
 * */

void bmfs_table_end(struct BMFSTable *table);

/** Goes to the previous entry in the table.
 * @param table An initialized table structure.
 * @returns A pointer to the previous table entry.
 * If the table is already at the beginning of
 * the table, then @ref BMFS_NULL is returned.
 * @ingroup table-api
 * */

struct BMFSTableEntry *bmfs_table_previous(struct BMFSTable *table);

/** Goes to the next entry in the table.
 * @param table An initialized table structure.
 * @returns A pointer to the next table entry.
 * If the table is already at the last entry,
 * then @ref BMFS_NULL is returned.
 * @ingroup table-api
 * */

struct BMFSTableEntry *bmfs_table_next(struct BMFSTable *table);

/** Writes the current entry to the table.
 * @param table An initialized table structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_save(struct BMFSTable *table);

/** Saves all entries in the allocation table.
 * This function also performs the checksums for
 * each of the entries.
 * @param table An initialized table structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_save_all(struct BMFSTable *table);

/** Add an entry to the end of the allocation table.
 * @param table An initialized allocation table.
 * @param entry The entry to add to the table.
 * @returns Zero on success, an error code on failure.
 * @ingroup table-api
 * */

int bmfs_table_push(struct BMFSTable *table,
                    struct BMFSTableEntry *entry);

/** Sets the disk that the table is located on.
 * @param table An initialized table structure.
 * @param disk The disk to read the table from.
 * @ingroup table-api
 * */

void bmfs_table_set_disk(struct BMFSTable *table,
                         struct BMFSDisk *disk);

/** Sets the offset on the disk that the table begins at.
 * This is used by @ref bmfs_table_import, so this function
 * must be called before using that one.
 * @param table An initialized table structure.
 * @param offset The offset, in bytes, of the table on disk.
 * @ingroup table-api
 * */

void bmfs_table_set_offset(struct BMFSTable *table,
                           bmfs_uint64 offset);

/** Sets the number of entries in the allocation table.
 * This is required to be called by the owner of the table.
 * @param table An initialized table structure
 * @param count The number of entries in the table.
 * @ingroup table-api
 * */

void bmfs_table_set_count(struct BMFSTable *table,
                          bmfs_uint64 count);

/** Set the maximum offset that an allocation may
 * reach. This is effectively the disk size.
 * @param table An initialized table structure.
 * @param max_offset The maximum offset, in bytes,
 * that an allocation may reach.
 * @ingroup table-api
 * */

void bmfs_table_set_max_offset(struct BMFSTable *table,
                               bmfs_uint64 max_offset);

/** Set the minimum offset allocation. This is
 * useful for protecting the data at the beginning of
 * the disk layout.
 * @param table An initialized table structure.
 * @param min_offset The minimum offset, in bytes, that
 * an allocation can have.
 * @ingroup table-api
 * */

void bmfs_table_set_min_offset(struct BMFSTable *table,
                               bmfs_uint64 min_offset);

/** Set the number of bytes per block.
 * The block size is used when creating an allocation.
 * Every allocation is a product of the block size.
 * @param table An initialized table structure.
 * @param block_size The number of bytes in one block.
 * @ingroup table-api
 * */

void bmfs_table_set_block_size(struct BMFSTable *table,
                               bmfs_uint64 block_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TABLE_H */

