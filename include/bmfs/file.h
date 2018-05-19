/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_FILE_H
#define BMFS_FILE_H

#include <bmfs/entry.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup file-api File API
 * Used for reading and writing file data.
 * */

struct BMFSDisk;

/** Indicates what operations are expected
 * to take place when the file is opened.
 * */

enum BMFSFileMode
{
	/** The file will be used for reading
	 * and writing. */
	BMFS_FILE_MODE_RW,
	/** The file will only be used for reading. */
	BMFS_FILE_MODE_READ,
	/** The file will only be used for writing. */
	BMFS_FILE_MODE_WRITE
};

/** This is a file structure that is used
 * to read and write file data on the file system.
 * @ingroup file-api
 * */

struct BMFSFile
{
	/** The disk that the file resides on. */
	struct BMFSDisk *Disk;
	/** A pointer to file the file allocation table. */
	struct BMFSTable *Table;
	/** The file's directory entry. This is
	 * used to update modification time and
	 * to stay within boundaries of the file
	 * region. */
	struct BMFSEntry Entry;
	/** The current position of the next read
	 * or write operation. */
	bmfs_uint64 CurrentPosition;
	/** The number of bytes reserved for the
	 * file to grow. */
	bmfs_uint64 ReservedSize;
	/** The mode associated with the this file. */
	enum BMFSFileMode Mode;
};

/** Initializes a file structure.
 * After calling this function, @ref bmfs_file_set_disk
 * should be called so that the file has a disk to read
 * from or write to.
 * @param file The file to initialize.
 * @ingroup file-api
 * */

void bmfs_file_init(struct BMFSFile *file);

/** Closes the file. If the file was opened for
 * writing, this function updates the file size
 * and modification time.
 * @param file An initialized file structure.
 * @ingroup file-api
 * */

void bmfs_file_close(struct BMFSFile *file);

/** Indicates whether or not the end of the file
 * position has been reached.
 * @param file An initialized file structure.
 * @returns Zero if the end of file has not been
 * eached, one if it has been reached.
 * @ingroup file-api
 * */

int bmfs_file_eof(const struct BMFSFile *file);

/** Reduces the size of the file to zero.
 * This function only works if the file is
 * in writing mode.
 * @param file An initialized file structure.
 * @ingroup file-api
 * */

void bmfs_file_truncate(struct BMFSFile *file);

/** Assigns the disk that will be used
 * to read and write file data, as well
 * as import the meta-data on the file itself.
 * Call @ref bmfs_file_import to get the file
 * information from disk.
 * @param file An initialized file structure.
 * @param disk The disk to assign to the file.
 * @ingroup file-api
 * */

void bmfs_file_set_disk(struct BMFSFile *file,
                        struct BMFSDisk *disk);

/** Sets the mode for the file.
 * This affects which IO operations will
 * work and which ones wont.
 * @param file An initialized file structure.
 * @param mode The mode to assign the file.
 * @ingroup file-api
 * */

void bmfs_file_set_mode(struct BMFSFile *file,
                        enum BMFSFileMode mode);

/** Imports a file from the current location on disk.
 * @param file An initialized file structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup file-api
 * */

int bmfs_file_import(struct BMFSFile *file);

/** Reads data from a file.
 * For this function to work, the file
 * must be open for reading.
 * @param file An initialized file structure.
 * @param buf The buffer to put the data into.
 * @param buf_size The number of bytes to read.
 * @param read_result The number of bytes that
 * were actually read from the disk.
 * @returns Zero on success, an error code on failure.
 * @ingroup file-api
 * */

int bmfs_file_read(struct BMFSFile *file,
                   void *buf,
                   bmfs_uint64 buf_size,
                   bmfs_uint64 *read_result);

/** Writes data to the file.
 * For this to work, the file must be open for writing.
 * @param file An initialized file structure.
 * @param buf The buffer containing the data to write.
 * @param buf_size The number of bytes to write to the file.
 * @param write_result The number of bytes that were
 * actually written to the file.
 * @returns Zero on success, an error code on failure.
 * @ingroup file-api
 * */

int bmfs_file_write(struct BMFSFile *file,
                    const void *buf,
                    bmfs_uint64 buf_size,
                    bmfs_uint64 *write_result);

/** Set the position of the next read or write operation.
 * @param file An initialized file structure.
 * @param pos The next read or write position.
 * @param whence Whether to start at the beginning or end
 * of file. See @ref BMFS_SEEK_SET and @ref BMFS_SEEK_END.
 * @returns Zero on success, an error code on failure.
 * @ingroup file-api
 * */

int bmfs_file_seek(struct BMFSFile *file,
                   bmfs_uint64 pos,
                   int whence);

/** Get the position of the next read or write operation.
 * @param file An initialized file structure.
 * @param pos A pointer to a variable that will receive
 * the position of the next read or write operation.
 * @returns Zero on success, an error code on failure.
 * @ingroup file-api
 * */

int bmfs_file_tell(struct BMFSFile *file,
                   bmfs_uint64 *pos);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_FILE_H */
