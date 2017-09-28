#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bmfs/path.h>

static void test1(void)
{
	int err;
	char path_str[] = "usr/include";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == 0);
	assert(root.Length == (sizeof("usr") - 1));
	assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	assert(path.Length == (sizeof("include") - 1));
	assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
}

static void test2(void)
{
	int err;
	char path_str[] = "/usr/include";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == 0);
	assert(root.Length == (sizeof("usr") - 1));
	assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	assert(path.Length == (sizeof("include") - 1));
	assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
}

static void test3(void)
{
	int err;
	char path_str[] = "//usr/include";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == 0);
	assert(root.Length == (sizeof("usr") - 1));
	assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	assert(path.Length == (sizeof("include") - 1));
	assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
}

static void test4(void)
{
	int err;
	char path_str[] = "\\Program Files\\BMFS";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == 0);
	assert(root.Length == (sizeof("Program Files") - 1));
	assert(memcmp(root.String, "Program Files", (size_t) root.Length) == 0);
	assert(path.Length == (sizeof("BMFS") - 1));
	assert(memcmp(path.String, "BMFS", (size_t) path.Length) == 0);
}

int main(void)
{
	test1();
	test2();
	test3();
	test4();
	return EXIT_SUCCESS;
}
