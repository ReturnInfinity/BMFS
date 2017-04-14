#include "sspec.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

static int test_string(const char *str, uint64_t value);

#define test(a, b) assert(test_string(a, b) == 0)

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
	return EXIT_SUCCESS;
}

static int test_string(const char *str, uint64_t value)
{
	struct bmfs_sspec sspec;

	int err = bmfs_sspec_parse(&sspec, str);
	if (err != 0)
		return err;

	uint64_t actual_value;
	err = bmfs_sspec_bytes(&sspec, &actual_value);
	if (err != 0)
		return err;

	if (value != actual_value)
		return -EINVAL;

	return 0;
}

