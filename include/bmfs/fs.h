/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_FS_H
#define BMFS_FS_H

#include <bmfs/header.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup fs-api File System API
 * Used for reading and manipulating
 * the file system as a whole.
 * */

struct BMFSDir;
struct BMFSDisk;
struct BMFSFile;

/** Represents the file system as a whole.
 * @ingroup fs-api
 * */

struct BMFS
{
	/** The disk used for read and
	 * write operations. This must
	 * be set by the caller. */
	struct BMFSDisk *Disk;
	/** The file system header. */
	struct BMFSHeader Header;
};

/** Initializes the file system.
 * After calling this, @ref bmfs_set_disk
 * should be called.
 * @param bmfs An uninitialized BMFS structure.
 * @ingroup fs-api
 * */

void bmfs_init(struct BMFS *bmfs);

/** Assigns the disk to use with a
 * BMFS file system structure. After
 * calling this, the file system should
 * either be formated with @ref bmfs_format
 * or imported with @ref bmfs_import.
 * @param bmfs An initialized file system structure.
 * @param disk The disk to find or format the file system on.
 * @ingroup fs-api
 * */

void bmfs_set_disk(struct BMFS *bmfs,
                   struct BMFSDisk *disk);

/** Locates a region that can
 * fit a certain number of bytes.
 * @param fs An initialized file system structure.
 * @param bytes The number of bytes that must be allocated.
 * @param offset A pointer that will receive the offset, in
 * bytes, of the new allocation.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup fs-api
 */

int bmfs_allocate(struct BMFS *fs,
                  bmfs_uint64 bytes,
                  bmfs_uint64 *offset);

/** Locates a region that can fit a certain number of mebibytes.
 * @param fs An initialized file system structure.
 * @param mebibytes The number of mebibytes that must be allocated.
 * @param offset A pointer that will receive the offset, in bytes,
 * of the new allocation.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup fs-api
 */

int bmfs_allocate_mebibytes(struct BMFS *fs,
                            bmfs_uint64 mebibytes,
                            bmfs_uint64 *offset);

/** Checks for the file system signature.
 * This is useful for determining wether or
 * not the disk has a valid BMFS file system.
 * @param fs An initialized file system structure.
 * @returns Zero on success, an error code on failure.
 * */

int bmfs_check_signature(struct BMFS *fs);

/** Formats the disk with the file system.
 * This should only be called if the disk
 * does not currently have a file system on it.
 * @param bmfs An initialized file system structure.
 * @param size The number of bytes to reserve for the filesystem.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_format(struct BMFS *bmfs, bmfs_uint64 size);

/** Imports a BMFS file system from the disk.
 * @param bmfs An initialized file system structure.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_import(struct BMFS *bmfs);

/** Creates a file on the file system.
 * @param bmfs An initialized file system structure.
 * @param path The path of the file to create. The
 * parent directory, if it has one, must exist. This
 * string must be null-terminated.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_create_file(struct BMFS *bmfs,
                     const char *path);

/** Creates a directory on the file system.
 * @param bmfs An initialized file system structure.
 * @param path The path to the directory to create.
 * The parent directory, if it has one, must exist.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_create_dir(struct BMFS *bmfs,
                    const char *path);

/** Opens a file on the file system. The file must
 * exist for this function to open the file. This
 * function has no way of knowing if the file is already
 * open - that is up to the caller.
 * @param bmfs An initialized file system structure.
 * @param file An initialized file structure.
 * @param path The path of the file to open.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_open_file(struct BMFS *bmfs,
                   struct BMFSFile *file,
                   const char *path);

/** Opens a directory on the file system. The
 * directory must already exist in order to
 * open it. This function does not check if the
 * directory is already open, that is up to the
 * caller.
 * @param bmfs An initialized file system structure.
 * @param dir An initialized directory structure.
 * @param path The path to the directory.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_open_dir(struct BMFS *bmfs,
                  struct BMFSDir *dir,
                  const char *path);

/** Deletes a file on the file system.
 * @param bmfs An initialized file system structure.
 * @param path The path of the file to delete.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_delete_file(struct BMFS *bmfs,
                     const char *path);

/** Deletes a directory on the file system.
 * The directory is only deleted if the contents are empty.
 * @param bmfs An initialized file system structure.
 * @param path The path to the directory.
 * @returns Zero on success, an error code on failure.
 * If the directory is not empty, then @ref BMFS_ENOTEMPTY
 * is returned.
 * @ingroup fs-api
 * */

int bmfs_delete_dir(struct BMFS *bmfs,
                    const char *path);

/** Deletes a directory and everything that the directory contains.
 * @param bmfs An initialized file system.
 * @param path The path of the directory to delete.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_delete_dir_recursively(struct BMFS *bmfs,
                                const char *path);

/** Rename a file or directory.
 * The parent directories of both paths
 * must exist.
 * @param bmfs An initialized file system.
 * @param old_path The path of the existing entry.
 * @param new_path The path to move the existing entry to.
 * @returns Zero on success, an error code on failure.
 * @ingroup fs-api
 * */

int bmfs_rename(struct BMFS *bmfs,
                const char *old_path,
                const char *new_path);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_FS_H */
