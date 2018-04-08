#include <bmfs/table.h>
#include <bmfs/ramdisk.h>
#include <bmfs/limits.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"

void test_alloc(void)
{
	/* Allocate enough room for the file system
	 * data and three allocations. */

	bmfs_uint64 block_size = 512;

	bmfs_uint64 table_size = sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_COUNT_MAX;

	bmfs_uint64 mem_size = 0;
	mem_size += block_size * 6;
	mem_size += table_size;

	void *mem = malloc(mem_size);
	bmfs_assert(mem != NULL);

	struct BMFSRamdisk ramdisk;

	bmfs_ramdisk_init(&ramdisk);

	bmfs_ramdisk_set_buf(&ramdisk, mem, mem_size);

	/* Setup the table */

	struct BMFSTable table;

	bmfs_table_init(&table);

	bmfs_table_set_block_size(&table, block_size);

	bmfs_table_set_disk(&table, &ramdisk.base);

	bmfs_table_set_offset(&table, 0x00);

	bmfs_table_set_count(&table, 0x00);

	bmfs_table_set_min_offset(&table, table_size);

	bmfs_table_set_max_offset(&table, mem_size);

	bmfs_uint64 offset1 = 0;

	int err = bmfs_table_alloc(&table, block_size / 2, &offset1);
	bmfs_assert(err == 0);

	bmfs_uint64 offset2 = 0;

	err = bmfs_table_alloc(&table, block_size * 2, &offset2);
	bmfs_assert(err == 0);

	bmfs_uint64 offset3 = 0;

	err = bmfs_table_alloc(&table, block_size / 2, &offset3);
	bmfs_assert(err == 0);

	bmfs_assert(offset2 == (offset1 + block_size));
	bmfs_assert(offset3 == (offset1 + (block_size * 3)));
}

int main(void)
{
	test_alloc();
	return EXIT_SUCCESS;
}
