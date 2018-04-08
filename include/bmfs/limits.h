/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

/** @file */

#ifndef BMFS_LIMITS_H
#define BMFS_LIMITS_H

/** This is the size, including padding,
 * of the file system header.
 * */

#define BMFS_HEADER_SIZE 512ULL

/** This is the size of a single entry,
 * in bytes.
 * */

#define BMFS_ENTRY_SIZE 256ULL

/** This is the maximum number of characters
 * that may be occupied by a file name
 * in BMFS. This includes the null terminating
 * character.
 * */

#define BMFS_FILE_NAME_MAX 192ULL

/** When directories or files are created,
 * a certain amount of space is reserved for
 * them to grow. This amount of reserved space
 * is counted in 'blocks'. This is the size of
 * one block.
 * */

#define BMFS_BLOCK_SIZE (1024ULL * 1024ULL * 2ULL)

/** This is the minimum size, in bytes, that a
 * disk must be to have BMFS formatted on it. This
 * number is not exactly calculated, it's just what
 * seems like enough to fit the meta data and one or
 * two allocations.
 * */

#define BMFS_MINIMUM_DISK_SIZE (BMFS_BLOCK_SIZE * 3ULL)

/** The size of a single entry in the allocation table.
 * */

#define BMFS_TABLE_ENTRY_SIZE 24ULL

#endif /* BMFS_LIMITS_H */

