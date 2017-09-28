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

static void test5(void)
{
	int err;
	char path_str[] = "/empty-base/";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == 0);
	assert(root.Length == (sizeof("empty-base") - 1));
	assert(memcmp(root.String, "empty-base", (size_t) path.Length) == 0);
	assert(path.Length == 0);
}

static void test_failure1(void)
{
	int err;
	char path_str[] = "no-root";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == -1);
	/* Ensure the path wasn't modified */
	assert(path.Length == (sizeof("no-root") - 1));
	assert(memcmp(path.String, "no-root", (size_t) path.Length) == 0);
}

static void test_failure2(void)
{
	int err;
	char path_str[] = "/no-root";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == -1);
	/* Ensure the path wasn't modified */
	assert(path.Length == (sizeof("/no-root") - 1));
	assert(memcmp(path.String, "/no-root", (size_t) path.Length) == 0);
}

static void test_failure3(void)
{
	int err;
	char path_str[] = "/";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	assert(err == -1);
	/* Ensure the string wasn't modified */
	assert(path.Length == 1);
	assert(path.String[0] == '/');
	assert(path.String[1] == 0);
}

int main(void)
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test_failure1();
	test_failure2();
	test_failure3();
	return EXIT_SUCCESS;
}
