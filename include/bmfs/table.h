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
	/** The number of bytes used by the region. */
	bmfs_uint64 Used;
	/** The number of bytes reserved for the region. */
	bmfs_uint64 Reserved;
	/** Flags associated with the allocation. */
	bmfs_uint64 Flags;
};

/** Initializes a table entry.
 * @param entry The table entry to initialize.
 * @ingroup table-api
 * */

void bmfs_table_entry_init(struct BMFSTableEntry *entry);

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

/** The disk allocation table.
 * Used for allocation space on the disk
 * and ensuring that no two allocations
 * overlap.
 * @ingroup table-api
 * */

struct BMFSTable {
	/** A pointer to the associated disk. */
	struct BMFSDisk *Disk;
	/** A placeholder for the current entry. */
	struct BMFSTableEntry CurrentEntry;
	/** The offset of the table on disk. */
	bmfs_uint64 TableOffset;
	/** The index of the current table entry. */
	bmfs_uint64 EntryIndex;
	/** If true, ignore entries that are deleted. */
	bmfs_bool IgnoreDeleted;
};

/** Initializes an allocation table.
 * Call @ref bmfs_table_set_disk and @ref bmfs_table_set_offset
 * before using any other functions.
 * @param table The allocation table to initialize.
 * @ingroup table-api
 * */

void bmfs_table_init(struct BMFSTable *table);

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
 * @returns The first entry in the table.
 * @ingroup table-api
 * */

struct BMFSTableEntry *bmfs_table_begin(struct BMFSTable *table);

/** Goes to the end of the table.
 * @param table An initialized table structure.
 * @returns The last entry in the table.
 * @ingroup table-api
 * */

struct BMFSTableEntry *bmfs_table_end(struct BMFSTable *table);

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

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TABLE_H */

