/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#ifndef BMFS_DISK_H
#define BMFS_DISK_H

#include "entry.h"
#include "dir.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup disk-api Disk API
 * Read and write from a BMFS formatted
 * disk.
 */

/** An abstract disk structure.
 * This structure allows a disk
 * to be represented by anything
 * that can read and write data.
 * @ingroup disk-api
 */

struct BMFSDisk
{
	/** A pointer used by the disk implementation to
	 * pass to the seek, tell, read and write methods.
	 */
	void *disk;
	/** Called when the disk is no longer going to be
	 * used. */
	void (*done)(void *disk);
	/** Points the disk to a particular location.
	 */
	int (*seek)(void *disk, int64_t offset, int whence);
	/** Retrieves the current location of the disk.
	 */
	int (*tell)(void *disk, int64_t *offset);
	/** Reads data from the disk.
	 */
	int (*read)(void *disk, void *buf, uint64_t len, uint64_t *read_len);
	/** Writes data to the disk.
	 */
	int (*write)(void *disk, const void *buf, uint64_t len, uint64_t *write_len);
};

/** Initializes the members of the disk structure
 * for pointer safety. This function must be called
 * before any others.
 * @ingroup disk-api
 * */

void bmfs_disk_init(struct BMFSDisk *disk);

/** Called when the disk is no longer going
 * to be used by the caller.
 * @ingroup disk-api
 * */

void bmfs_disk_done(struct BMFSDisk *disk);

/** Points the disk to a particular offset.
 * @param disk An initialized disk.
 * @param offset The offset to point the disk to.
 * @param whence Where the base the offset from.
 *  This value may be SEEK_SET, SEEK_CUR or SEEK_END.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup disk-api
 */

int bmfs_disk_seek(struct BMFSDisk *disk,
                   int64_t offset,
                   int whence);

/** Retrieves the current offset of the disk.
 * @param disk An initialized disk.
 * @param offset A pointer to the variable that
 *  will receive the disk offset.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_tell(struct BMFSDisk *disk,
                   int64_t *offset);

/** Reads data from the disk.
 * @param disk An initialized disk.
 * @param buf Where to put the data
 *  read from the disk.
 * @param len The number of bytes
 *  available in @p buf.
 * @param read_len A pointer to the
 *  variable that will receive the
 *  number of bytes read from the
 *  disk. This field may be NULL.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_read(struct BMFSDisk *disk,
                   void *buf,
                   uint64_t len,
                   uint64_t *read_len);

/** Writes data to the disk.
 * @param disk An initialized disk.
 * @param buf Contains the data to
 *  be written to disk.
 * @param len The number of bytes in
 *  @p buf to write to the disk.
 * @param write_len A pointer to the
 *  variable that will receive the
 *  number of bytes written to disk.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_write(struct BMFSDisk *disk,
                    const void *buf,
                    uint64_t len,
                    uint64_t *write_len);

/** Determines the amount of bytes
 * available in the disk.
 * @param disk An initialized disk.
 * @param bytes A pointer to the
 *  variable that will receive the
 *  number of bytes available on
 *  the disk.
 * @returns Zero on success, a
 *  negative error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_bytes(struct BMFSDisk *disk,
                    uint64_t *bytes);

/** Determines the number of mebibytes
 * available on disk.
 * @param disk An initialized disk.
 * @param mebibytes A pointer to the
 *  variable that will receive the
 *  number of mebibytes on the disk.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_mebibytes(struct BMFSDisk *disk,
                        uint64_t *mebibytes);

/** Determines the number of blocks
 * available on disk.
 * @param disk An initialized disk.
 * @param blocks A pointer to the
 *  variable that will receive the
 *  number of blocks on the disk.
 * @returns Zero on success, a
 *  negative error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_blocks(struct BMFSDisk *disk,
                     uint64_t *blocks);

/** Locates a region that can
 * fit a certain number of bytes.
 * @param disk An initialized disk.
 * @param bytes The number of bytes
 *  that must be allocated.
 * @param offset A pointer that will
 * receive the offset, in bytes, of the
 * new allocation.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_allocate(struct BMFSDisk *disk,
                       uint64_t bytes,
                       uint64_t *offset);

/** Locates a region that can
 * fit a certain number of mebibytes.
 * @param disk An initialized disk.
 * @param mebibytes The number of
 *  mebibytes that must be allocated.
 * @param offset A pointer that will
 * receive the offset, in bytes, of the
 * new allocation.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_allocate_mebibytes(struct BMFSDisk *disk,
                                 uint64_t mebibytes,
                                 uint64_t *offset);

/** Checks for the file system signature.
 * This is useful for determining wether or
 * not the disk has a valid BMFS file system.
 * @param disk An initialized disk structure.
 * @returns Zero on success, an error code on
 * failure.
 * */

int bmfs_disk_check_signature(struct BMFSDisk *disk);

/** Locates a file entry.
 * @param disk An initialized disk.
 * @param filename The name of the file
 *  to search for.
 * @param entry A pointer to an entry
 *  structure that will receive all the
 *  file information if the file is found.
 * @param number A pointer to a variable
 *  that will receive the index of the file
 *  in the root directory, if it is found.
 *  This parameter may be NULL.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_find_file(struct BMFSDisk *disk,
                        const char *filename,
                        struct BMFSEntry *entry,
                        uint64_t *number);

/** Reads the root directory on disk.
 * @param disk An initialized disk.
 * @param dir A pointer to a directory
 *  structure that will receive all the
 *  entries in the root directory.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_read_root_dir(struct BMFSDisk *disk,
                            struct BMFSDir *dir);

/** Writes to the root directory.
 * All previous entries in the root
 * directory are replaced.
 * @param disk An initialized disk.
 * @param dir The directory to write
 *  to the disk.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_write_root_dir(struct BMFSDisk *disk,
                             const struct BMFSDir *dir);

/** Creates a file on the disk.
 * This function fails if the file
 * exists or if there isn't enough
 * space to create the file.
 * @param disk An initialized disk.
 * @param filename The name of the
 *  new file entry.
 * @param mebibytes The number of
 *  mebibytes to reserve for the
 *  new file.
 * @returns Zero on success, a
 *  negative error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_create_file(struct BMFSDisk *disk,
                          const char *filename,
                          uint64_t mebibytes);

/** Creates a directory on the disk.
 * @param disk An initialized disk structure.
 * @param dirname The name of the directory to create.
 *  If this directory already exists, the function will fail.
 * @returns Zero on success, a negative error code otherwise.
 * @ingroup disk-api
 */

int bmfs_disk_create_dir(struct BMFSDisk *disk,
                         const char *dirname);

/** Deletes a file from the disk.
 * If the file doesn't exist, this
 * function fails.
 * @param disk An initialized disk.
 * @param filename The name of the
 *  file to delete.
 * @returns Zero on success, a negative
 *  error code on failure.
 * @ingroup disk-api
 */

int bmfs_disk_delete_file(struct BMFSDisk *disk,
                          const char *filename);

/** Formats the disk so that the file
 * system can function properly. This
 * function will clear all existing
 * allocations if the disk is already
 * formatted to BMFS.
 * @param disk An initialized disk.
 * @param size The number of bytes to allow
 * for the file system to grow.
 * @returns Zero on success, a negative
 *  error code on failure.
 */

int bmfs_disk_format(struct BMFSDisk *disk, uint64_t size);

/** Reads content of a specified file.
 * @param disk An initialized disk.
 * @param filename The name of the entry to read from.
 * @param buf The data to put the file content into.
 * @param len The number of bytes to read.
 * @param off The offset within the file data to read.
 * @returns Zero on success, a negative error code
 *  on failure.
 * @ingroup disk-api
 */

int bmfs_read(struct BMFSDisk *disk,
              const char *filename,
              void * buf,
              uint64_t len,
              uint64_t off);

/** Writes contents to a specified file.
 * @param disk An initialized disk.
 * @param filename The name of the entry to write to.
 * @param buf The data to write to the file.
 * @param len The number of bytes to write from @p buf.
 * @param off The offset within the file to begin writing.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup disk-api
 */

int bmfs_write(struct BMFSDisk *disk,
               const char *filename,
               const void *buf,
               uint64_t len,
               uint64_t off);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_DISK_H */

