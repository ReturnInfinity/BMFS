/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/path.h>

#include <stdlib.h>

static int is_separator(char c)
{
	if ((c == '/')
	 || (c == '\\'))
		return 1;
	else
		return 0;
}

void bmfs_path_init(struct BMFSPath *path)
{
	path->String = NULL;
	path->Length = 0;
}

int bmfs_path_empty(const struct BMFSPath *path)
{
	return path->String == 0;
}

int bmfs_path_split_root(struct BMFSPath *path,
                         struct BMFSPath *root)
{
	uint64_t i;
	uint64_t path_len;
	const char *path_str;

	path_str = path->String;
	path_len = path->Length;

	if (path_len == 0) {
		root->String = NULL;
		root->Length = 0;
	}

	i = 0;

	root->String = &path_str[i];
	root->Length = 0;
	while (i < path_len)
	{
		if (is_separator(path_str[i])) {
			if (root->Length == 0) {
				i++;
				root->String = &path_str[i];
				continue;
			} else {
				/* The +1 is for the '/' or '\' separator */
				path->String = &path->String[i + 1];
				path->Length -= i + 1;
				/* root dir found */
				return 0;
			}
		} else {
			root->Length++;
		}
		i++;
	}

	path->String = &path->String[i];
	path->Length -= i;

	return 0;
}

void bmfs_path_set(struct BMFSPath *path,
                   const char *string,
                   uint64_t length)
{
	path->String = string;
	path->Length = length;
}
