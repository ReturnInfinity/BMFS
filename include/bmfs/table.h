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

/** The disk allocation table.
 * Used for allocation space on the disk
 * and ensuring that no two allocations
 * overlap.
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
};

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TABLE_H */

