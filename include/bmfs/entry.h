/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_ENTRY_H
#define BMFS_ENTRY_H

#include <bmfs/limits.h>

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSDisk;

/** @defgroup entry-api Entry API
 * Examine and manipulate file entry data.
 */

/** Indicates the type of entry. This
 * may be a directory or a file.
 * @ingroup entry-api
 */

enum BMFSEntryType
{
	/** The entry is a file. */
	BMFS_TYPE_FILE,
	/** The entry is a directory. */
	BMFS_TYPE_DIRECTORY,
	/** The entry is a link to another entry. */
	BMFS_TYPE_LINK
};

/** An entry within a BMFS directory.
 * Contains information on a file, such
 * as name, size and more.
 * @ingroup entry-api
 */

struct BMFSEntry
{
	/** The name of the entry. */
	char Name[BMFS_FILE_NAME_MAX];
	/** The offset of the entry data, in bytes. */
	uint64_t Offset;
	/** The number of bytes used by the entry data. */
	uint64_t Size;
	/** The time that the entry was created. */
	uint64_t CreationTime;
	/** The time that the entry was last modified. */
	uint64_t ModificationTime;
	/** Stores information on entry type
	 * and permissions. */
	uint64_t Flags;
	/** The owner user ID */
	uint64_t UserID;
	/** The group ID */
	uint64_t GroupID;
	/** The offset of the entry within it's parent directory. */
	uint64_t EntryOffset;
};

/** Initializes an entry.
 * @param entry An uninitialized entry.
 * @ingroup entry-api
 */

void bmfs_entry_init(struct BMFSEntry *entry);

/** Copies the contents of an entry structure
 * from one location to another.
 * @param dst The destination entry.
 * @param src The source entry. This is
 * the entry that contains the data that
 * is copied over to the destination entry.
 * @ingroup entry-api
 * */

void bmfs_entry_copy(struct BMFSEntry *dst,
                     const struct BMFSEntry *src);

/** Reads an entry from disk.
 * @param entry The entry to read.
 * @param disk The disk to read the entry from.
 * @returns Zero on success, an error code on failure.
 * @ingroup entry-api
 * */

int bmfs_entry_read(struct BMFSEntry *entry,
                    struct BMFSDisk *disk);

/** Writes an entry to disk.
 * @param entry The entry to write.
 * @param disk The disk to write the entry to.
 * @returns Zero on success, an error code on failure.
 * @ingroup entry-api
 * */

int bmfs_entry_write(const struct BMFSEntry *entry,
                     struct BMFSDisk *disk);

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

/** Sets the type of entry.
 * @param entry An initialized entry.
 * @param entry_type The type of entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_type(struct BMFSEntry *entry,
                         enum BMFSEntryType entry_type);

/** Indicates wether or not the
 * entry is a directory.
 * @param entry An initialized entry.
 * @returns One if the entry is a directory,
 *  zero if it is not.
 * @ingroup entry-api
 */

int bmfs_entry_is_directory(const struct BMFSEntry *entry);

/** Indicates wether or not the
 * entry is a file.
 * @param entry An initialized entry.
 * @returns One if the entry is a file,
 *  zero if it is not.
 * @ingroup entry-api
 */

int bmfs_entry_is_file(const struct BMFSEntry *entry);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_ENTRY_H */

