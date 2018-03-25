#include <assert.h>
#include <bmfs/size.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
	test("0B", 0ULL);
	test("0",  0ULL);
	test("31G", 31ULL * 1024ULL * 1024ULL * 1024ULL);
	return EXIT_SUCCESS;
}

static int test_string(const char *str, uint64_t value)
{
	struct BMFSSize sspec;

	int err = bmfs_size_parse(&sspec, str);
	if (err != 0)
		return err;

	uint64_t actual_value;
	err = bmfs_size_bytes(&sspec, &actual_value);
	if (err != 0)
		return err;

	if (value != actual_value)
		return -EINVAL;

	return 0;
}

