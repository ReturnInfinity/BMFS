/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

/** @file */

#ifndef BMFS_TABLE_H
#define BMFS_TABLE_H

/** @defgroup table-api Table
 * Examine and manipulate the disk space
 * allocation table.
 */

/** Indicates the maximum number of entries
 * in the disk space allocation table.
 * @ingroup table-api
 * */

#define BMFS_TABLE_ENTRY_COUNT_MAX 1024

#include <stdint.h>

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
	uint64_t Offset;
	/** The number of bytes used by the region. */
	uint64_t Used;
	/** The number of bytes reserved for the region. */
	uint64_t Reserved;
};

void bmfs_table_entry_init(struct BMFSTableEntry *entry);

/** Reads a table entry from the current position on the disk.
 * @param entry The structure to put the entry data into.
 * @param disk The disk to read the entry from.
 * @returns Zero on success, an error code on failure.
 * */

int bmfs_table_entry_read(struct BMFSTableEntry *entry,
                          struct BMFSDisk *disk);

/** Writes a table entry to disk at the current location.
 * @param entry The entry to write to disk.
 * @param disk The disk to write the entry to.
 * @returns Zero on success, an error code on failure.
 * */

int bmfs_table_entry_write(const struct BMFSTableEntry *entry,
                           struct BMFSDisk *disk);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TABLE_H */

