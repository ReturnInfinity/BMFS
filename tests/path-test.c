#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bmfs/path.h>

#include "assert.h"

static void test1(void)
{
	int err;
	char path_str[] = "usr/include";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("usr") - 1));
	bmfs_assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	bmfs_assert(path.Length == (sizeof("include") - 1));
	bmfs_assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
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
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("usr") - 1));
	bmfs_assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	bmfs_assert(path.Length == (sizeof("include") - 1));
	bmfs_assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
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
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("usr") - 1));
	bmfs_assert(memcmp(root.String, "usr", (size_t) root.Length) == 0);
	bmfs_assert(path.Length == (sizeof("include") - 1));
	bmfs_assert(memcmp(path.String, "include", (size_t) path.Length) == 0);
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
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("Program Files") - 1));
	bmfs_assert(memcmp(root.String, "Program Files", (size_t) root.Length) == 0);
	bmfs_assert(path.Length == (sizeof("BMFS") - 1));
	bmfs_assert(memcmp(path.String, "BMFS", (size_t) path.Length) == 0);
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
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("empty-base") - 1));
	bmfs_assert(memcmp(root.String, "empty-base", (size_t) path.Length) == 0);
	bmfs_assert(path.Length == 0);
}

static void test6(void) {

	int err;
	char path_str[] = "/tmp";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("tmp") - 1));
	bmfs_assert(memcmp(root.String, "tmp", (size_t) path.Length) == 0);
	bmfs_assert(path.Length == 0);
}

static void test_failure1(void)
{
	int err;
	char path_str[] = "no-slash";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	bmfs_assert(err == 0);
	bmfs_assert(root.Length == (sizeof("no-slash") - 1));
	bmfs_assert(memcmp(root.String, "no-slash", (size_t) path.Length) == 0);
	bmfs_assert(path.Length == 0);
}

static void test_failure2(void)
{
	int err;
	char path_str[] = "/";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_split_root(&path, &root);
	bmfs_assert(err == 0);
	bmfs_assert(path.Length == 0);
	bmfs_assert(root.Length == 0);
}

#if 0
struct VisitorData {
	unsigned int name_index;
};

static void visit_parent(void *data_ptr, const char *name, uint64_t name_size)
{
	struct VisitorData *data = (struct VisitorData *) data_ptr;

	bmfs_assert(data->name_index <= 1);

	if (data->name_index == 0) {
		bmfs_assert(name_size == 2);
		bmfs_assert(memcmp(name, "ab", 3) == 0);
	} else if (data->name_index == 1) {
		bmfs_assert(name_size == 3);
		bmfs_assert(memcmp(name, "cde", 3) == 0);
	}
}

static void visit_basename(void *data_ptr, const void *name, uint64_t name_size)
{
	struct VisitorData *data = (struct VisitorData *) data_ptr;

	bmfs_assert(data->name_index == 2);

	bmfs_assert(name_size == 2);
	bmfs_assert(memcmp(name, "fg", 2) == 0);
}

static void test_visitor(void)
{

	struct BMFSPath path;

	bmfs_path_init(&path);

	bmfs_path_set(&path, "/ab/cde/fg", sizeof("/ab/cde/fg") - 1);

	struct VisitorData visitor_data;
	visitor_data.name_index = 0;

	struct BMFSPathVisitor visitor;
	visitor.data = &visitor_data;
	visitor.visit_parent = visit_parent;
	visitor.visit_basename = visit_basename;

	int ret = bmfs_path_visit(&path, &visitor);
	bmfs_assert(ret == 0);
}

#endif

int main(void)
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test_failure1();
	test_failure2();
	return EXIT_SUCCESS;
}
