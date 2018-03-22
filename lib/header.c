/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.2.3 (2017 04 07) */

#include <bmfs/header.h>

#include <bmfs/disk.h>
#include <bmfs/limits.h>
#include <bmfs/table.h>

#include <errno.h>

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
}

int bmfs_header_read(struct BMFSHeader *header,
                     struct BMFSDisk *disk)
{
	uint64_t read_size = 0;

	int err = bmfs_disk_read(disk, header, sizeof(*header), &read_size);
	if (err != 0)
		return err;
	else if (read_size != sizeof(*header))
		return -EIO;

	return 0;
}

int bmfs_header_write(const struct BMFSHeader *header,
                      struct BMFSDisk *disk)
{
	uint64_t write_size = 0;

	int err = bmfs_disk_write(disk, header, sizeof(*header), &write_size);
	if (err != 0)
		return err;
	else if (write_size != sizeof(*header))
		return -EIO;

	return 0;
}
