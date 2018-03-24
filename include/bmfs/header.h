/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_HEADER_H
#define BMFS_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSDisk;

/** Describes various positions and
 * characteristics of the file system.
 * It is the first structure in the
 * layout.
 * */

struct BMFSHeader
{
	/** The BMFS file system tag.
	 * If this is not set to 'BMFS',
	 * then the file system is not
	 * BMFS formatted. */
	char Signature[8];
	/** The offset, in bytes, of the root directory.*/
	uint64_t RootOffset;
	/** The offset, in bytes, of the allocation table. */
	uint64_t TableOffset;
	/** The number of used entries in the allocation table. */
	uint64_t TableEntryCount;
	/** The total size allowed for the file system to grow. */
	uint64_t TotalSize;
	/** Padding until the 512-byte marker.
	 * This space is alos reserved for future use. */
	uint64_t Reserved[59];
};

void bmfs_header_init(struct BMFSHeader *header);

int bmfs_header_read(struct BMFSHeader *header,
                     struct BMFSDisk *disk);

int bmfs_header_write(const struct BMFSHeader *header,
                      struct BMFSDisk *disk);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_HEADER_H */

