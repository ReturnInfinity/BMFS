/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#ifndef BMFS_HEADER_H
#define BMFS_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct BMFSHeader
{
	/** The BMFS file system tag.
	 * If this is not set to 'BMFS',
	 * then the file system is not
	 * BMFS formatted. */
	char tag[4];
	/** The version of the BMFS
	 * file system being used. */
	unsigned char version;
	/** Information held by headers
	 * greater than or equal to version
	 * one. */
	struct {
		/** The offset of the root directory,
		 * relative to the end of the BMFS
		 * header. */
		uint64_t root_offset;
		/** The number of bytes that the file system
		 * is allowed to occupy, not including the size
		 * of this header. */
		uint64_t size;
	} v1_info;
};

void bmfs_header_init(struct BMFSHeader *header);

int bmfs_header_read(struct BMFSHeader *header,
                     const void *buf,
                     uint64_t buf_size);

int bmfs_header_write(struct BMFSHeader *header,
                      void *buf,
                      uint64_t buf_size);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_HEADER_H */

