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
#include <bmfs/types.h>

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
	BMFS_TYPE_LINK,
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
	bmfs_uint64 Offset;
	/** The number of bytes used by the entry data. */
	bmfs_uint64 Size;
	/** The time that the entry was created. */
	bmfs_uint64 CreationTime;
	/** The time that the entry was last modified. */
	bmfs_uint64 ModificationTime;
	/** Stores information on entry type,
	 * permissions, and state. */
	bmfs_uint64 Flags;
	/** The owner user ID */
	bmfs_uint64 UserID;
	/** The group ID */
	bmfs_uint64 GroupID;
	/** The offset of the entry within it's parent directory. */
	bmfs_uint64 EntryOffset;
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

/** Indicates whether or not the entry
 * has been deleted.
 * @param entry An initialized entry structure.
 * @returns @ref BMFS_TRUE if the entry has been
 * deleted, @ref BMFS_FALSE if it has not been.
 * @ingroup entry-api
 * */

bmfs_bool bmfs_entry_is_deleted(const struct BMFSEntry *entry);

/** Reads an entry from disk.
 * @param entry The entry to read.
 * @param disk The disk to read the entry from.
 * @returns Zero on success, an error code on failure.
 * @ingroup entry-api
 * */

int bmfs_entry_read(struct BMFSEntry *entry,
                    struct BMFSDisk *disk);

/** Writes the entry, at its offset, to disk.
 * The entry must have been read from disk, using
 * the function @ref bmfs_entry_read, because the
 * this function needs a location to write the entry
 * to. If the entry just needs to be written to disk
 * at the current location, use @ref bmfs_disk_write.
 * @param entry An initialized entry structure.
 * @param disk The disk to write the entry to.
 * @returns Zero on success, an error code on failure.
 * @ingroup entry-api
 * */

int bmfs_entry_save(struct BMFSEntry *entry,
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
                          bmfs_uint64 *offset);

/** Indicate that the entry is deleted.
 * @param entry An initialized entry structure.
 * @ingroup entry-api
 * */

void bmfs_entry_set_deleted(struct BMFSEntry *entry);

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
                              bmfs_uint64 file_size);

/** Sets the starting block of the entry.
 * @param entry An initialized or
 *  uninitialized entry.
 * @param starting_block The new starting
 *  block of the entry.
 * @ingroup entry-api
 */

void bmfs_entry_set_starting_block(struct BMFSEntry *entry,
                                   bmfs_uint64 starting_block);

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

