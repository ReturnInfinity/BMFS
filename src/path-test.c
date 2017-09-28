#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bmfs/path.h>

static void test1(void) {

	int err;
	char path_str[] = "usr/include";
	struct BMFSPath path;
	struct BMFSPath root;

	bmfs_path_init(&path);
	bmfs_path_set(&path, path_str, sizeof(path_str) - 1);

	err = bmfs_path_root(&path, &root);
	assert(err == 0);
	assert(root.Length == 3);
	assert(memcmp(root.String, "usr", 3) == 0);
}

int main(void) {
	test1();
	return EXIT_SUCCESS;
}
