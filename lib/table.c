#include <bmfs/table.h>

#include <bmfs/disk.h>

#include <errno.h>

void bmfs_table_entry_init(struct BMFSTableEntry *entry)
{
	entry->Offset = 0;
	entry->Used = 0;
	entry->Reserved = 0;
}

int bmfs_table_entry_read(struct BMFSTableEntry *entry,
                          struct BMFSDisk *disk)
{
	uint64_t read_count = 0;

	int err = bmfs_disk_read(disk, entry, sizeof(*entry), &read_count);
	if (err != 0)
		return err;
	else if (read_count != sizeof(*entry))
		return -EIO;

	return 0;
}

int bmfs_table_entry_write(const struct BMFSTableEntry *entry,
                           struct BMFSDisk *disk)
{
	uint64_t write_count = 0;

	int err = bmfs_disk_write(disk, entry, sizeof(*entry), &write_count);
	if (err != 0)
		return err;
	else if (write_count != sizeof(*entry))
		return -EIO;

	return 0;
}
