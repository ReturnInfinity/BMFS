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

static void test_realloc(void)
{
	bmfs_uint64 block_size = 512;

	bmfs_uint64 table_size = sizeof(struct BMFSTableEntry) * BMFS_TABLE_ENTRY_COUNT_MAX;

	bmfs_uint64 mem_size = 0;
	mem_size += block_size * 6;
	mem_size += table_size;

	unsigned char *mem = malloc(mem_size);
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

	int err = bmfs_table_alloc(&table, block_size, &offset1);
	bmfs_assert(err == 0);

	bmfs_uint64 offset2 = 0;

	err = bmfs_table_alloc(&table, block_size * 2, &offset2);
	bmfs_assert(err == 0);

	/* Ensure that the second allocation follows
	 * the first one, immediately. */

	bmfs_assert(offset2 == (offset1 + block_size));

	/* We're going to test re-allocation now.
	 * First, we'll put data into the block we
	 * are resizing, so we know whether or not
	 * it is copied over. */

	mem[offset1 + 0] = 'm';
	mem[offset1 + 1] = 'a';
	mem[offset1 + 2] = 'g';
	mem[offset1 + 3] = 'i';
	mem[offset1 + 4] = 'c';
	mem[offset1 + 5] = 0;

	mem[offset1 + (block_size - 7)] = 'n';
	mem[offset1 + (block_size - 6)] = 'u';
	mem[offset1 + (block_size - 5)] = 'm';
	mem[offset1 + (block_size - 4)] = 'b';
	mem[offset1 + (block_size - 3)] = 'e';
	mem[offset1 + (block_size - 2)] = 'r';
	mem[offset1 + (block_size - 1)] = 0;

	/* Re-allocate the first block to two blocks.
	 * This will cause the table to have to put it
	 * after the second allocation. */

	err = bmfs_table_realloc(&table, block_size * 2, &offset1);
	bmfs_assert(err == 0);

	/* Ensure that the new allocation immediately follows
	 * the second allocation. */
	bmfs_assert(offset1 == (offset2 + (block_size * 2)));

	/* Ensure that all the data was copied over. */

	/* The first bits */
	bmfs_assert(memcmp(&mem[offset1], "magic", sizeof("magic")) == 0);
	/* The last bits */
	bmfs_assert(memcmp(&mem[offset1 + (block_size - 7)], "number", sizeof("number")) == 0);

	bmfs_table_done(&table);
}

int main(void)
{
	test_alloc();
	test_realloc();
	return EXIT_SUCCESS;
}
