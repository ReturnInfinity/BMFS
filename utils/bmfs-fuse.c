/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/bmfs.h>
#include <bmfs/filedisk.h>

#define FUSE_USE_VERSION 30

#include <fuse.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>
#include <fcntl.h>

/** The file system structure.
 * Since FUSE can't pass data
 * to the callback functions,
 * this will have to suffice.
 * */

struct BMFS fs;

/** These are options read from
 * the command line. */

struct bmfs_fuse_options
{
	/** The path of the disk file */
	const char *disk;
	/** A flag set when help is requested */
	int show_help;
};

#define BMFS_FUSE_OPTION(t, p) \
    { t, offsetof(struct bmfs_fuse_options, p), 1 }
static const struct fuse_opt option_spec[] = {
	BMFS_FUSE_OPTION("--disk=%s", disk),
	BMFS_FUSE_OPTION("-h", show_help),
	BMFS_FUSE_OPTION("--help", show_help),
	FUSE_OPT_END
};

/** Called when the fuse connection
 * is initialized. Not currently used.
 * */

static void *bmfs_fuse_init(struct fuse_conn_info *conn)
{
	(void) conn;
	return NULL;
}

static int bmfs_fuse_access(const char *path, int mode)
{
	(void) mode;

	if (S_ISDIR(mode))
	{
		struct BMFSDir dir;

		bmfs_dir_init(&dir);

		return bmfs_open_dir(&fs, &dir, path);
	}

	if (S_ISREG(mode))
	{
		struct BMFSFile file;

		bmfs_file_init(&file);

		return bmfs_open_file(&fs, &file, path);
	}

	/* Unsupported file type */

	return -ENOSYS;
}

/** Gets permissions and size of a
 * specific file in the directory.
 * Currently, all files in BMFS have
 * read and write permissions only.
 * */

static int bmfs_fuse_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));

	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	int err = bmfs_open_dir(&fs, &dir, path);
	if (err == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	struct BMFSFile file;

	bmfs_file_init(&file);

	err = bmfs_open_file(&fs, &file, path);
	if (err == 0)
	{
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = file.Entry.Size;
		return 0;
	}

	return -ENOENT;
}

/** This function does not actually
 * do anything, since BMFS does not
 * support time stamps.
 * */

static int bmfs_fuse_utimens(const char *path, const struct timespec tv[2])
{
	(void) path;
	(void) tv;
	return 0;
}

/** Lists contents of a directory.
 * Since BMFS only has one directory,
 * this function basically lists all
 * files in the file system.
 * */

static int bmfs_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                             off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	/* standard directory entries */
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	/* get the current directory entries */

	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	int err = bmfs_open_dir(&fs, &dir, path);
	if (err != 0)
		return -ENOENT;

	for (;;)
	{
		const struct BMFSEntry *entry = bmfs_dir_next(&dir);
		if (entry == NULL)
			break;

		filler(buf, entry->Name, NULL, 0);
	}

	return 0;
}

/** Creates a file, defaulting to the
 * size of 2 MiB.
 * */

static int bmfs_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	(void) fi;

	if (S_ISREG(mode))
		return bmfs_create_file(&fs, path);
	else if (S_ISDIR(mode))
		return bmfs_create_dir(&fs, path);
	else
		return -ENOSYS;
}

/** Deletes a file.
 * */

static int bmfs_fuse_unlink(const char *path)
{
	return bmfs_delete_file(&fs, path);
}

/** This function opens a file.
 * Except, the way fuse is implemented,
 * it really just checks that it exists.
 * */

static int bmfs_fuse_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;

	struct BMFSFile file;

	bmfs_file_init(&file);

	int err = bmfs_open_file(&fs, &file, path);
	if (err == 0)
		return err;

	return 0;
}

/** Reads data from a file.
 * The data is written to the buf parameter.
 * The file is read starting at the offset
 * parameter.
 * @returns The number of bytes read.
 * */

static int bmfs_fuse_read(const char *path, char *buf, size_t size, off_t offset,
                          struct fuse_file_info *fi)
{
	(void) fi;

	struct BMFSFile file;

	bmfs_file_init(&file);

	int err = bmfs_open_file(&fs, &file, path);
	if (err != 0)
		return err;

	err = bmfs_file_seek(&file, offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 read_count = 0;

	err = bmfs_file_read(&file, buf, size, &read_count);
	if (err != 0)
		return err;

	return read_count;
}

/** Writes data to a file.
 * This function's parameters are similar
 * to that of bmfs_fuse_read, except that
 * it cannot return zero.
 * @returns The number of bytes written.
 *  If zero is returned, it is considered
 *  an error.
 * */

static int bmfs_fuse_write(const char *path, const char *buf, size_t size, off_t offset,
                           struct fuse_file_info *fi)
{
	(void) fi;

	struct BMFSFile file;

	bmfs_file_init(&file);

	int err = bmfs_open_file(&fs, &file, path);
	if (err != 0)
		return err;

	err = bmfs_file_seek(&file, offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 write_count = 0;

	err = bmfs_file_write(&file, buf, size, &write_count);
	if (err != 0)
		return err;

	return write_count;
}

static struct fuse_operations bmfs_fuse_operations = {
	.init = bmfs_fuse_init,
	.access = bmfs_fuse_access,
	.getattr = bmfs_fuse_getattr,
	.utimens = bmfs_fuse_utimens,
	.readdir = bmfs_fuse_readdir,
	.create = bmfs_fuse_create,
	.unlink = bmfs_fuse_unlink,
	.open = bmfs_fuse_open,
	.read = bmfs_fuse_read,
	.write = bmfs_fuse_write
};

static void show_help(const char *argv0)
{
	fprintf(stderr, "usage: %s <mountpoint> [options]\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "BMFS Options:\n");
	fprintf(stderr, "    --disk=<s>             The disk file to mount (defaults to 'disk.image')\n");
	fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	struct bmfs_fuse_options options = {
		/* .disk may be reallocated, can't
		 * use string literal */
		.disk = strdup("bmfs.img"),
		.show_help = 0
	};

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return EXIT_FAILURE;

	if (options.show_help)
	{
		show_help(argv[0]);

		/* this displays fuse-specific help and exits */
		if (fuse_opt_add_arg(&args, "-ho") != 0)
			return EXIT_FAILURE;

		/* allow fuse_main() to run, so that the help
		 * message is displayed */
		return fuse_main(args.argc, args.argv, &bmfs_fuse_operations, NULL);
	}

	/* Open the disk file. */

	struct BMFSFileDisk filedisk;

	bmfs_filedisk_init(&filedisk);

	int err = bmfs_filedisk_open(&filedisk, options.disk, "r+b");
	if (err != 0)
	{
		fprintf(stderr, "%s: Failed to open '%s': %s\n", argv[0], options.disk, strerror(errno));
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	/* Initialize the file system. */

	bmfs_init(&fs);

	bmfs_set_disk(&fs, &filedisk.base);

	err = bmfs_import(&fs);
	if (err != 0)
	{
		fprintf(stderr, "Error: Failed to import file system.\n");
		fprintf(stderr, "Reason: %s\n", strerror(-err));
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	int retval = fuse_main(args.argc, args.argv, &bmfs_fuse_operations, NULL);

	bmfs_filedisk_done(&filedisk);

	return retval;
}
