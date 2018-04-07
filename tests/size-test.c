#include <bmfs/size.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"

static int test_string(const char *str, bmfs_uint64 value);

#define test(a, b) bmfs_assert(test_string(a, b) == 0)

int main(void)
{
	test("4TiB", 4ULL * 1024ULL * 1024ULL * 1024ULL * 1024ULL);
	test("4TB",  4ULL * 1000ULL * 1000ULL * 1000ULL * 1000ULL);
	test("4GiB", 4ULL * 1024ULL * 1024ULL * 1024ULL);
	test("4GB",  4ULL * 1000ULL * 1000ULL * 1000ULL);
	test("3MiB", 3ULL * 1024ULL * 1024ULL);
	test("3MB",  3ULL * 1000ULL * 1000ULL);
	test("2KiB", 2ULL * 1024ULL);
	test("2KB",  2ULL * 1000ULL);
	test("1B", 1ULL);
	test("1",  1ULL);
	test("0B", 0ULL);
	test("0",  0ULL);
	test("31G", 31ULL * 1024ULL * 1024ULL * 1024ULL);
	test("512B", 512);
	return EXIT_SUCCESS;
}

static int test_string(const char *str, bmfs_uint64 value)
{
	struct BMFSSize sspec;

	int err = bmfs_size_parse(&sspec, str);
	if (err != 0)
		return err;

	bmfs_uint64 actual_value;
	err = bmfs_size_bytes(&sspec, &actual_value);
	if (err != 0)
		return err;

	if (value != actual_value)
		return -EINVAL;

	return 0;
}

