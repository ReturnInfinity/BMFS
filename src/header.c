/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include <bmfs/header.h>

void bmfs_header_init(struct BMFSHeader *header)
{
	header->tag[0] = 'B';
	header->tag[1] = 'M';
	header->tag[2] = 'F';
	header->tag[3] = 'S';
	header->version = 1;
	header->v1_info.root_offset = 0;
	header->v1_info.size = 0;
}

int bmfs_header_read(struct BMFSHeader *header,
                     const void *buf,
                     uint64_t buf_size)
{
	(void) header;
	(void) buf;
	(void) buf_size;
	return 0;
}
int bmfs_header_write(struct BMFSHeader *header,
                      void *buf,
                      uint64_t buf_size)
{
	(void) header;
	(void) buf;
	(void) buf_size;
	return 0;
}
