/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/header.h>

#include <bmfs/disk.h>
#include <bmfs/encoding.h>
#include <bmfs/errno.h>
#include <bmfs/limits.h>
#include <bmfs/table.h>

#include "crc32.h"

/* Header size calculation:
 *
 *  8 8 signature
 * 16 8 root offset
 * 24 8 table offset
 * 32 8 table entry count
 * 40 8 total size
 * 48 8 block size
 * 52 4 flags
 * 56 4 checksum
 *
 * Total size: 56
 */

#define BMFS_HEADER_SIZE 56

void bmfs_header_init(struct BMFSHeader *header)
{
	header->Signature[0] = 'B';
	header->Signature[1] = 'M';
	header->Signature[2] = 'F';
	header->Signature[3] = 'S';
	header->Signature[4] = 0;
	header->Signature[5] = 0;
	header->Signature[6] = 0;
	header->Signature[7] = 0;

	header->TableOffset = 0;
	header->TableOffset += sizeof(struct BMFSHeader);

	header->RootOffset = 0;
	header->RootOffset += header->TableOffset;
	header->RootOffset += BMFS_TABLE_ENTRY_COUNT_MAX * sizeof(struct BMFSTableEntry);

	header->TableEntryCount = 0;
	header->TotalSize = BMFS_MINIMUM_DISK_SIZE;
	header->BlockSize = 4096;
	header->Flags = 0;
	header->Checksum = 0;
}

int bmfs_header_check(const struct BMFSHeader *header)
{
	if ((header->Signature[0] != 'B')
	 || (header->Signature[1] != 'M')
	 || (header->Signature[2] != 'F')
	 || (header->Signature[3] != 'S'))
	{
		return BMFS_EINVAL;
	}

	return 0;
}

int bmfs_header_read(struct BMFSHeader *header,
                     struct BMFSDisk *disk)
{
	unsigned char buf[BMFS_HEADER_SIZE];

	bmfs_uint64 read_size = 0;

	int err = bmfs_disk_read(disk, buf, sizeof(buf), &read_size);
	if (err != 0)
		return err;
	else if (read_size != sizeof(buf))
		return BMFS_EIO;

	header->Signature[0] = buf[0];
	header->Signature[1] = buf[1];
	header->Signature[2] = buf[2];
	header->Signature[3] = buf[3];

	header->Signature[4] = buf[4];
	header->Signature[5] = buf[5];
	header->Signature[6] = buf[6];
	header->Signature[7] = buf[7];

	header->RootOffset = bmfs_decode_uint64(&buf[8]);
	header->TableOffset = bmfs_decode_uint64(&buf[16]);
	header->TableEntryCount = bmfs_decode_uint64(&buf[24]);
	header->TotalSize = bmfs_decode_uint64(&buf[32]);
	header->BlockSize = bmfs_decode_uint64(&buf[40]);
	header->Flags = bmfs_decode_uint32(&buf[48]);
	header->Checksum = bmfs_decode_uint32(&buf[52]);

	buf[52] = 0;
	buf[53] = 0;
	buf[54] = 0;
	buf[55] = 0;

	bmfs_uint32 checksum = bmfs_crc32(0, buf, sizeof(buf));

	if (checksum != header->Checksum)
		return BMFS_EINVAL;

	return 0;
}

int bmfs_header_write(const struct BMFSHeader *header,
                      struct BMFSDisk *disk)
{
	unsigned char buf[BMFS_HEADER_SIZE];

	buf[0] = header->Signature[0];
	buf[1] = header->Signature[1];
	buf[2] = header->Signature[2];
	buf[3] = header->Signature[3];

	buf[4] = header->Signature[4];
	buf[5] = header->Signature[5];
	buf[6] = header->Signature[6];
	buf[7] = header->Signature[7];

	bmfs_encode_uint64(header->RootOffset, &buf[8]);
	bmfs_encode_uint64(header->TableOffset, &buf[16]);
	bmfs_encode_uint64(header->TableEntryCount, &buf[24]);
	bmfs_encode_uint64(header->TotalSize, &buf[32]);
	bmfs_encode_uint64(header->BlockSize, &buf[40]);
	bmfs_encode_uint32(header->Flags, &buf[48]);
	bmfs_encode_uint32(0, &buf[52]);

	bmfs_uint32 checksum = bmfs_crc32(0, buf, sizeof(buf));

	bmfs_encode_uint32(checksum, &buf[52]);

	bmfs_uint64 write_size = 0;

	int err = bmfs_disk_write(disk, buf, sizeof(buf), &write_size);
	if (err != 0)
		return err;
	else if (write_size != sizeof(buf))
		return BMFS_EIO;

	return 0;
}
