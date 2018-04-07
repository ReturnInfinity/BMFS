/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/bmfs.h>
#include <bmfs/filedisk.h>
#include <bmfs/size.h>

#define FUSE_USE_VERSION 30

#include <fuse.h>

#include <errno.h>
#include <stdarg.h>
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

/** This file is used for logging.
 * It may or may not be opened, depending
 * if logging was enabled on the command line.
 * */

struct BMFSFile log_file;

static int bmfs_log_size(const char *fmt, va_list args)
{
	FILE *null = fopen("/dev/null", "wb");
	if (null == NULL)
		return -1;

	int write_count = vfprintf(null, fmt, args);
	if (ferror(null))
		return -1;

	fclose(null);

	return write_count;
}

static void bmfs_log(const char *func, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	int log_size = bmfs_log_size(fmt, args);
	if (log_size < 0)
	{
		va_end(args);
		return;
	}

	va_end(args);

	char *tmp = malloc(log_size + 1);

	va_start(args, fmt);

	int write_count = vsnprintf(tmp, log_size + 1, fmt, args);
	if (write_count > (log_size + 1))
	{
		/* Hopefully this doesn't happen! */
		return;
	}

	/* Ensure a null terminator. */
	tmp[write_count] = 0;

	bmfs_file_write(&log_file, func, strlen(func), NULL);
	bmfs_file_write(&log_file, ": ", 2, NULL);
	bmfs_file_write(&log_file, tmp, write_count, NULL);

	va_end(args);
}

/** These are options read from
 * the command line. */

struct bmfs_fuse_options
{
	/** The path of the disk file */
	const char *disk;
	/** The offet to the file system on disk. */
	const char *offset;
	/** Allow the fuse driver to log operations. */
	int allow_logging;
	/** The path to the log, if logging is enabled. */
	const char *log_path;
	/** A flag set when help is requested */
	int show_help;
};

#define BMFS_FUSE_OPTION(t, p) \
    { t, offsetof(struct bmfs_fuse_options, p), 1 }
static const struct fuse_opt option_spec[] = {
	BMFS_FUSE_OPTION("--disk=%s", disk),
	BMFS_FUSE_OPTION("--offset=%s", offset),
	BMFS_FUSE_OPTION("--allow-logging", allow_logging),
	BMFS_FUSE_OPTION("--log-path", log_path),
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
	struct BMFSEntry entry;

	/* Try to open it as file. */

	struct BMFSFile file;

	bmfs_file_init(&file);

	int err = bmfs_open_file(&fs, &file, path);
	if (err == BMFS_ENOENT)
	{
		return -ENOENT;
	}
	else if (err == BMFS_EISDIR)
	{
		/* The entry is a directory. */

		struct BMFSDir dir;

		bmfs_dir_init(&dir);

		err = bmfs_open_dir(&fs, &dir, path);
		if (err != 0)
		{
			bmfs_log(__func__, "Failed to open directory '%s'.\n", path);
			return -ENOENT;
		}

		bmfs_entry_copy(&entry, &dir.Entry);
	}
	else if (err == 0)
	{
		bmfs_entry_copy(&entry, &file.Entry);
	}
	else
	{
		bmfs_log(__func__, "Failed to open file '%s'.\n", path);
		return -ENOENT;
	}

	if (mode & R_OK)
	{
		bmfs_log(__func__, "Not checking read permission.\n");
	}

	if (mode & W_OK)
	{
		bmfs_log(__func__, "Not checking write permission.\n");
	}

	if (mode & X_OK)
	{
		bmfs_log(__func__, "Not checking execute permission.\n");
	}

	return 0;
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
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = dir.Entry.ModificationTime;
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
		stbuf->st_atime = time(NULL);
		stbuf->st_mtime = file.Entry.ModificationTime;
		return 0;
	}

	return -ENOENT;
}

static int bmfs_fuse_utimens(const char *path, const struct timespec tv[2])
{
	/* TODO */
	(void) path;
	(void) tv;
	bmfs_log(__func__, "Function not implemented.\n");
	return 0;
}

static int bmfs_fuse_readlink(const char *path, char *buf, size_t buf_size)
{
	bmfs_log(__func__, "Function not implemented.\n");
	bmfs_log(__func__, "Failing to read '%s'.\n", path);
	(void) buf;
	(void) buf_size;
	return -ENOSYS;
}

static int bmfs_fuse_rename(const char *old_path, const char *new_path)
{
	int err = bmfs_rename(&fs, old_path, new_path);
	if (err == BMFS_ENOENT)
		return -ENOENT;
	else if (err == BMFS_ENOSPC)
		return -ENOSPC;
	else if (err == BMFS_ENOTDIR)
		return -ENOTDIR;
	else if (err == BMFS_EISDIR)
		return -EISDIR;
	else if (err == BMFS_EPERM)
		return -EPERM;

	return 0;
}

static int bmfs_fuse_rmdir(const char *path)
{
	int err = bmfs_delete_dir(&fs, path);
	if (err == BMFS_ENOENT)
		return -ENOENT;
	else if (err == BMFS_ENOTDIR)
		return -ENOTDIR;
	else if (err == BMFS_ENOTEMPTY)
		return -ENOTEMPTY;
	else
		bmfs_log(__func__, "Unhandled error '%s'\n", bmfs_strerror(err));
	
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

	int err = 0;

	if (S_ISREG(mode))
		err = bmfs_create_file(&fs, path);
	else if (S_ISDIR(mode))
		err = bmfs_create_dir(&fs, path);
	else
	{
		bmfs_log(__func__, "Unsupported file mode: %x\n", mode);
		return -ENOSYS;
	}

	if (err == BMFS_EEXIST)
		return -EEXIST;
	else if (err == BMFS_ENOENT)
		return -ENOENT;

	return 0;
}

/** Deletes a file.
 * */

static int bmfs_fuse_unlink(const char *path)
{
	int err = bmfs_delete_file(&fs, path);
	if (err == BMFS_ENOENT)
		return -ENOENT;

	return 0;
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

	if (fi->flags & O_RDONLY)
		bmfs_file_set_mode(&file, BMFS_FILE_MODE_READ);

	err = bmfs_file_seek(&file, offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 read_count = 0;

	err = bmfs_file_read(&file, buf, size, &read_count);
	if (err != 0)
		return err;

	bmfs_file_close(&file);

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

	if (fi->flags & O_RDONLY)
		bmfs_file_set_mode(&file, BMFS_FILE_MODE_READ);
	else if (fi->flags & O_WRONLY)
		bmfs_file_set_mode(&file, BMFS_FILE_MODE_WRITE);

	err = bmfs_file_seek(&file, offset, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	bmfs_uint64 write_count = 0;

	err = bmfs_file_write(&file, buf, size, &write_count);
	if (err != 0)
		return err;

	bmfs_file_close(&file);

	return write_count;
}

static int bmfs_fuse_symlink(const char *link, const char *path)
{
	bmfs_log(__func__, "Functionality not implemented.\n");
	bmfs_log(__func__, "Failed to create symbol link '%s' to '%s'.\n", link, path);
	return -ENOSYS;
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
	.write = bmfs_fuse_write,
	.readlink = bmfs_fuse_readlink,
	.rename = bmfs_fuse_rename,
	.rmdir = bmfs_fuse_rmdir,
	.symlink = bmfs_fuse_symlink
};

static void show_help(const char *argv0)
{
	fprintf(stderr, "usage: %s <mountpoint> [options]\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "BMFS Options:\n");
	fprintf(stderr, "    --disk=<s>             The disk file to mount (defaults to 'disk.image')\n");
	fprintf(stderr, "    --offset=<o>           The offset of the file system on disk (defaults to 512).\n");
	fprintf(stderr, "    --allow-logging        Allow for error conditions to be logged on a BMFS file.\n");
	fprintf(stderr, "    --log-path             If logging is enabled, log to this path on the BMFS image.\n");
	fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	struct bmfs_fuse_options options = {
		/* .disk may be reallocated, can't
		 * use string literal */
		.disk = strdup("bmfs.img"),
		.offset = strdup("512"),
		.allow_logging = 0,
		.log_path = strdup("fuse-log.txt"),
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
		fprintf(stderr, "%s: Failed to open '%s': %s\n", argv[0], options.disk, bmfs_strerror(err));
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	const char *offset_string = options.offset;
	if (offset_string == NULL)
		offset_string = "512";

	struct BMFSSize offset;

	if (bmfs_size_parse(&offset, offset_string) != 0)
	{
		fprintf(stderr, "%s: Failed to parse size '%s'.\n", argv[0], offset_string);
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	bmfs_uint64 offset_bytes = 0;

	if (bmfs_size_bytes(&offset, &offset_bytes) != 0)
	{
		fprintf(stderr, "%s: Offset '%s' is not representable by 64-bit integer.\n", argv[0], offset_string);
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	bmfs_filedisk_set_offset(&filedisk, offset_bytes);

	/* Initialize the file system. */

	bmfs_init(&fs);

	bmfs_set_disk(&fs, &filedisk.base);

	err = bmfs_import(&fs);
	if (err != 0)
	{
		fprintf(stderr, "Error: Failed to import file system.\n");
		fprintf(stderr, "Reason: %s\n", bmfs_strerror(err));
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	/* Open the log, if logging was enabled. */

	const char *log_path = options.log_path;
	if (log_path == NULL)
		log_path = "fuse-log.txt";

	bmfs_file_init(&log_file);

	if (options.allow_logging)
	{
		printf("Opening log at '%s'.\n", log_path);

		bmfs_create_file(&fs, log_path);

		bmfs_open_file(&fs, &log_file, log_path);

		bmfs_file_set_mode(&log_file, BMFS_FILE_MODE_WRITE);

		bmfs_file_truncate(&log_file);
	}

	int retval = fuse_main(args.argc, args.argv, &bmfs_fuse_operations, NULL);

	if (options.allow_logging)
	{
		bmfs_file_close(&log_file);
	}

	bmfs_filedisk_done(&filedisk);

	return retval;
}
