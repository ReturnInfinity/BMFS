/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/bmfs.h>
#include <bmfs/size.h>
#include <bmfs/filedisk.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef BMFS_DEFAULT_DISK_SIZE
#define BMFS_DEFAULT_DISK_SIZE (128 * 1024 * 1024)
#endif

/** Enumerates a list of commands
 * that the utility supports.
 * */

enum bmfs_command {
	/** No command specified. */
	BMFS_CMD_NONE,
	/** Unknown command */
	BMFS_CMD_UNKNOWN,
	/** Concatenate files to standard output. */
	BMFS_CMD_CAT,
	/** Copy a file from one location to another. */
	BMFS_CMD_CP,
	/** List directory contents */
	BMFS_CMD_LS,
	/** Format a BMFS image. */
	BMFS_CMD_FORMAT,
	/** Make a directory */
	BMFS_CMD_MKDIR,
	/** Create a file and update modification time. */
	BMFS_CMD_TOUCH,
	/** Remove a file */
	BMFS_CMD_RM,
	/** Remove an empty directory */
	BMFS_CMD_RMDIR,
	/** Print version */
	BMFS_CMD_VERSION,
	/** Print help contents of a command. */
	BMFS_CMD_HELP
};

static enum bmfs_command command_parse(const char *cmd)
{
	if (cmd == NULL)
		return BMFS_CMD_NONE;
	else if (strcmp(cmd, "cat") == 0)
		return BMFS_CMD_CAT;
	else if ((strcmp(cmd, "cp") == 0)
	      || (strcmp(cmd, "copy") == 0))
		return BMFS_CMD_CP;
	else if ((strcmp(cmd, "ls") == 0)
	      || (strcmp(cmd, "list") == 0)
	      || (strcmp(cmd, "dir") == 0))
		return BMFS_CMD_LS;
	else if (strcmp(cmd, "format") == 0)
		return BMFS_CMD_FORMAT;
	else if (strcmp(cmd, "mkdir") == 0)
		return BMFS_CMD_MKDIR;
	else if ((strcmp(cmd, "rm") == 0)
	      || (strcmp(cmd, "delete") == 0))
		return BMFS_CMD_RM;
	else if ((strcmp(cmd, "create") == 0)
	      || (strcmp(cmd, "touch") == 0))
		return BMFS_CMD_TOUCH;
	else if (strcmp(cmd, "rmdir") == 0)
		return BMFS_CMD_RMDIR;
	else if (strcmp(cmd, "version") == 0)
		return BMFS_CMD_VERSION;
	else if (strcmp(cmd, "help") == 0)
		return BMFS_CMD_HELP;

	return BMFS_CMD_UNKNOWN;
}

static void time_print(time_t time)
{
	struct tm ts;

	ts = *localtime(&time);

	char time_string[32];

	strftime(time_string, sizeof(time_string), "%b %d %H:%M:%S", &ts);

	printf("%s ", time_string);
}

static int is_opt(const char *arg,
                  char s_opt,
                  const char *l_opt)
{
	if (arg[0] != '-')
		return 0;
	else if ((arg[1] == s_opt)
	      && (arg[2] == 0))
		return 1;
	else if ((arg[1] == '-')
	      && (strcmp(&arg[2], l_opt) == 0))
		return 1;

	return 0;
}

static int file_exists(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		return 0;
	else
		return 1;
}

static int cmd_format(struct BMFS *bmfs, int argc, const char **argv);

static int cmd_mkdir(struct BMFS *bmfs, int argc, const char **argv);

static int cmd_cat(struct BMFS *bmfs, int argc, const char **argv);

static int cmd_cp(struct BMFS *bmfs, int argc, const char **argv);

static int cmd_ls(struct BMFS *bmfs, int argc, const char **argv);

static int cmd_touch(struct BMFS *bmfs, int argc, const char **argv);

static void print_help(const char *argv0, int argc, const char **argv);

static void print_version(void);

/* Program code */
int main(int argc, const char **argv)
{
	const char *diskname = "bmfs.img";

	int i = 1;

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-') {
			break;
		} else if (is_opt(argv[i], 'd', "disk")) {
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "Error: No disk path specified.\n");
				return EXIT_FAILURE;
			}
			diskname = argv[++i];
		} else {
			fprintf(stderr, "Error: Unknown option '%s'.\n", argv[i]);
			fprintf(stderr, "Run '%s help' for assistance.\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	enum bmfs_command cmd = command_parse(argv[i]);

	switch (cmd)
	{
	case BMFS_CMD_NONE:
		fprintf(stderr, "Error: No command specified.\n");
		fprintf(stderr, "Run '%s help' for assistance.\n", argv[0]);
		return EXIT_FAILURE;
	case BMFS_CMD_UNKNOWN:
		fprintf(stderr, "Error: Unknown command '%s'.\n", argv[i]);
		fprintf(stderr, "Run '%s help' for assistance.\n", argv[0]);
		return EXIT_FAILURE;
	case BMFS_CMD_HELP:
		print_help(argv[0], argc - i, &argv[i + 1]);
		return EXIT_FAILURE;
	case BMFS_CMD_VERSION:
		print_version();
		return EXIT_FAILURE;
	default:
		break;
	}

	i++;

	const char *mode = "rb+";

	if ((cmd == BMFS_CMD_FORMAT) && (!file_exists(diskname)))
		mode = "wb";

	struct BMFSFileDisk filedisk;

	bmfs_filedisk_init(&filedisk);

	int err = bmfs_filedisk_open(&filedisk, diskname, mode);
	if (err != 0)
	{
		printf("Error: Unable to open disk '%s'\n", diskname);
		printf("Reason: %s\n", strerror(-err));
		return EXIT_FAILURE;
	}

	struct BMFS bmfs;

	bmfs_init(&bmfs);

	bmfs_set_disk(&bmfs, &filedisk.base);

	if (cmd == BMFS_CMD_FORMAT)
	{
		int err = cmd_format(&bmfs, argc - i, &argv[i]);

		bmfs_filedisk_done(&filedisk);

		if (err != 0)
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	err = bmfs_import(&bmfs);
	if (err != 0)
	{
		fprintf(stderr, "Error: Failed to import file system.\n");
		fprintf(stderr, "Reason: %s\n", strerror(-err));
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	switch (cmd)
	{
	case BMFS_CMD_CAT:
		err = cmd_cat(&bmfs, argc - i, &argv[i]);
		break;
	case BMFS_CMD_CP:
		err = cmd_cp(&bmfs, argc - i, &argv[i]);
		break;
	case BMFS_CMD_LS:
		err =  cmd_ls(&bmfs, argc - i, &argv[i]);
		break;
	case BMFS_CMD_MKDIR:
		err = cmd_mkdir(&bmfs, argc - i, &argv[i]);
		break;
	case BMFS_CMD_TOUCH:
		err = cmd_touch(&bmfs, argc - i, &argv[i]);
		break;
	default:
		fprintf(stderr, "Error: Command not supported yet.\n");
		bmfs_filedisk_done(&filedisk);
		return EXIT_FAILURE;
	}

	bmfs_filedisk_done(&filedisk);

	if (err != 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int cmd_format(struct BMFS *bmfs, int argc, const char **argv)
{
	int i = 0;

	unsigned int force_flag = 0;

	struct BMFSSize disk_size;

	if (bmfs_size_parse(&disk_size, "64MiB") != 0)
	{
		fprintf(stderr, "Failed to parse default disk size.\n");
		return EXIT_FAILURE;
	}

	while (i < argc)
	{
		if (argv[i][0] != '-')
		{
			fprintf(stderr, "Error: Trailing argument: %s\n", argv[i]);
		}
		else if (is_opt(argv[i], 's', "size"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "Error: Disk size not specified.\n");
				return EXIT_FAILURE;
			}
			if (bmfs_size_parse(&disk_size, argv[i + 1]))
			{
				fprintf(stderr, "Error: Failed to parse disk size '%s'.\n", argv[i]);
				return EXIT_FAILURE;
			}
			i++;
		}
		else if (is_opt(argv[i], 'f', "force"))
		{
			force_flag = 1;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}

		i++;
	}

	uint64_t disk_byte_count = 0;

	if (bmfs_size_bytes(&disk_size, &disk_byte_count) != 0)
	{
		fprintf(stderr, "Error: Disk size too large.\n");
		return EXIT_FAILURE;
	}

	if (disk_byte_count == 0)
	{
		fprintf(stderr, "Error: Disk size must be greater than zero.\n");
		return EXIT_FAILURE;
	}

	int err = bmfs_check_signature(bmfs);
	if ((err == 0) && !force_flag)
	{
		fprintf(stderr, "Error: Disk already formatted.\n");
		fprintf(stderr, "Use '-f' or '--force' to override this error.\n");
		return EXIT_FAILURE;
	}

	err = bmfs_disk_seek(bmfs->Disk, disk_byte_count - 1, BMFS_SEEK_SET);
	if (err != 0)
		return err;

	err = bmfs_disk_write(bmfs->Disk, "\x00", 1, NULL);
	if (err != 0)
		return err;

	err = bmfs_format(bmfs, (uint64_t) disk_byte_count);
	if (err != 0)
		return err;

	return 0;
}

static int cmd_mkdir(struct BMFS *bmfs, int argc, const char **argv)
{
	int i = 0;

	while (i < argc)
	{
		if (argv[i][0] == '-') {
			fprintf(stderr, "Error: Options must be specified before directory paths.\n");
			return EXIT_FAILURE;
		}

		const char *path = argv[i];

		int err = bmfs_create_dir(bmfs, path);
		if (err != 0)
		{
			fprintf(stderr, "Error: Failed to create '%s'.\n", path);
			fprintf(stderr, "\t%s\n", strerror(-err));
			return EXIT_FAILURE;
		}

		i++;
	}

	return 0;
}

static int cmd_cat(struct BMFS *bmfs, int argc, const char **argv)
{
	int i = 0;

	char buf[512];

	while (i < argc)
	{
		struct BMFSFile file;

		bmfs_file_init(&file);

		int err = bmfs_open_file(bmfs, &file, argv[i]);
		if (err != 0)
		{
			fprintf(stderr, "Error: Failed to open '%s'.\n", argv[i]);
			fprintf(stderr, "Reason: %s\n", strerror(-err));
			bmfs_file_close(&file);
			return EXIT_FAILURE;
		}

		while (!bmfs_file_eof(&file))
		{
			uint64_t read_result = 0;

			err = bmfs_file_read(&file, buf, 512, &read_result);
			if (err != 0)
			{
				fprintf(stderr, "Error: Failed to read '%s'.\n", argv[i]);
				fprintf(stderr, "Reason: %s\n", strerror(-err));
				bmfs_file_close(&file);
				return EXIT_FAILURE;
			}

			size_t write_result = fwrite(buf, 1, read_result, stdout);
			if (write_result != read_result)
			{
				fprintf(stderr, "Error: Failed to write data to standard output.\n");
				bmfs_file_close(&file);
				return EXIT_FAILURE;
			}
		}

		bmfs_file_close(&file);

		i++;
	}

	return EXIT_SUCCESS;
}

static int cmd_cp(struct BMFS *bmfs, int argc, const char **argv)
{
	if (argc < 1)
	{
		fprintf(stderr, "Error: Missing source path.\n");
		return EXIT_FAILURE;
	}
	else if (argc < 2)
	{
		fprintf(stderr, "Error: Missing destination path.\n");
		return EXIT_FAILURE;
	}
	else if (argc > 2)
	{
		fprintf(stderr, "Error: Trailing arguments.\n");
		return EXIT_FAILURE;
	}

	const char *src_path = argv[0];

	FILE *src = fopen(src_path, "rb");
	if (src == NULL)
	{
		int err = errno;
		fprintf(stderr, "Error: Failed to open '%s' for reading.\n", src_path);
		fprintf(stderr, "Reason: %s\n", strerror(err));
		return EXIT_FAILURE;
	}

	const char *dst_path = argv[1];

	struct BMFSFile dst;

	bmfs_file_init(&dst);

	int err = bmfs_create_file(bmfs, dst_path);
	if ((err != 0) && (err != -EEXIST))
	{
		fprintf(stderr, "Failed to create '%s'.\n", dst_path);
		fprintf(stderr, "Reason: %s\n", strerror(-err));
		fclose(src);
		return EXIT_FAILURE;
	}

	err = bmfs_open_file(bmfs, &dst, dst_path);
	if (err != 0)
	{
		fprintf(stderr, "Failed to open '%s' for writing.\n", dst_path);
		fprintf(stderr ,"Reason: %s\n", strerror(-err));
		fclose(src);
		return EXIT_FAILURE;
	}

	bmfs_file_set_mode(&dst, BMFS_FILE_MODE_WRITE);

	size_t buf_size = 4096;

	void *buf = malloc(buf_size);
	if (buf == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for data transfer.\n");
		fclose(src);
		bmfs_file_close(&dst);
		return EXIT_FAILURE;
	}

	while (!feof(src))
	{
		size_t read_result = fread(buf, 1, buf_size, src);
		if (ferror(src))
		{
			err = errno;
			fprintf(stderr, "Error: Failed to read from '%s'.\n", src_path);
			fprintf(stderr, "Reason: %s\n", strerror(err));
			fclose(src);
			bmfs_file_close(&dst);
			free(buf);
			return EXIT_FAILURE;
		}

		uint64_t write_result = 0;

		err = bmfs_file_write(&dst, buf, read_result, &write_result);
		if ((err != 0) || (write_result != read_result))
		{
			if (err == 0)
				err = -EIO;

			fprintf(stderr, "Error: Failed to write to '%s'.\n", dst_path);
			fprintf(stderr, "Reason: %s\n", strerror(-err));
			fclose(src);
			bmfs_file_close(&dst);
			free(buf);
			return EXIT_FAILURE;
		}
	}

	free(buf);

	fclose(src);

	bmfs_file_close(&dst);

	return EXIT_SUCCESS;
}

static int cmd_ls(struct BMFS *bmfs, int argc, const char **argv)
{
	struct BMFSDir dir;

	bmfs_dir_init(&dir);

	int i = 0;

	const int color_always = 1;

	const int color_never = 0;

	int color_mode = color_always;

	const int info_name = 0;

	const int info_time = 0x01;

	const int info_size = 0x02;

	const int info_all = info_time | info_size;

	int info_mode = info_name;

	while (i < argc)
	{
		if (argv[i][0] != '-') {
			break;
		} else if (is_opt(argv[i], 'l', "long-list")) {
			info_mode = info_all;
		} else if (is_opt(argv[i], 'c', "color")) {
			if ((i + 1) >= argc) {
				fprintf(stderr, "Error: Color mode not specified.\n");
				return EXIT_FAILURE;
			} else if (strcmp(argv[i + 1], "always") == 0) {
				color_mode = color_always;
			} else if (strcmp(argv[i + 1], "never") == 0) {
				color_mode = color_never;
			} else if (strcmp(argv[i + 1], "auto") == 0) {
				fprintf(stderr, "Error: Auto coloring not supported.\n");
				return EXIT_FAILURE;
			} else {
				fprintf(stderr, "Error: Unknown coloring mode '%s'.\n", argv[i + 1]);
				return EXIT_FAILURE;
			}
			i++;
		} else {
			fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
			return EXIT_FAILURE;
		}

		i++;
	}

	const char *path = "/";

	if (i < argc)
		path = argv[i];

	int err = bmfs_open_dir(bmfs, &dir, path);
	if (err != 0)
	{
		fprintf(stderr, "Error: Failed to read '%s'.\n", path);
		fprintf(stderr, "Reason: %s\n", strerror(-err));
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < 64; i++)
	{
		const struct BMFSEntry *entry = bmfs_dir_next(&dir);
		if (entry == NULL)
			break;

		if ((info_mode & info_size) != 0)
			printf("%8llu ", (unsigned long long int) entry->Size);

		if ((info_mode & info_time) != 0)
			time_print(entry->ModificationTime);

		if (bmfs_entry_is_directory(entry) && (color_mode == color_always))
			printf("\033[34;1m%s\033[0m\n", entry->Name);
		else
			printf("%s\n", entry->Name);
	}

	return EXIT_SUCCESS;
}

static int cmd_touch(struct BMFS *bmfs, int argc, const char **argv)
{
	int i = 0;

	while (i < argc)
	{
		if (argv[i][0] != '-')
		{
			break;
		}
		else
		{
			fprintf(stderr, "Error: Unrecognized option '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	while (i < argc)
	{
		if (argv[i][0] == '-') {
			fprintf(stderr, "Error: Options must be specified before directory paths.\n");
			return EXIT_FAILURE;
		}

		int err = bmfs_create_file(bmfs, argv[i]);
		if (err != 0)
		{
			fprintf(stderr, "Failed to create '%s'.\n", argv[i]);
			fprintf(stderr, "Reason: %s\n", strerror(-err));
			return EXIT_FAILURE;
		}

		i++;
	}

	return EXIT_FAILURE;
}

static void print_usage(const char *argv0)
{
	printf("Usage: %s [options] <command>\n", argv0);
	printf("\n");
	printf("Options:\n");
	printf("\t-d, --disk PATH : Specify the path to the BMFS disk.\n");
	printf("\n");
	printf("Commands:\n");
	printf("\thelp    : Get help with a command.\n");
	printf("\tversion : Print version information and exit.\n");
	printf("\tls      : Lists entries in a specified directory.\n");
	printf("\tlist    : Alias for 'ls'.\n");
	printf("\tread    : Reads a file from the BMFS file system to the host file system.\n");
	printf("\twrite   : Writes a file from the host file system to BMFS file system.\n");
	printf("\ttouch   : Creates a file if it doesn't exist and updates its modification time.\n");
	printf("\trm      : Deletes a file if it exists.\n");
	printf("\tdelete  : Alias for 'rm'.\n");
	printf("\tformat  : Formats an existing file with BMFS.\n");
	printf("\tmkdir   : Creates a directory, if it doesn't exist.\n");
	printf("\n");
	printf("File: may be used in a read, write, create or delete operation\n");
}

static void print_help(const char *argv0, int argc, const char **argv)
{
	if (argc == 0)
	{
		print_usage(argv0);
		return;
	}

	enum bmfs_command cmd = command_parse(argv[0]);

	switch (cmd)
	{
	case BMFS_CMD_NONE:
		print_usage(argv0);
		break;
	case BMFS_CMD_UNKNOWN:
		fprintf(stderr, "Error: Unknown command '%s'.\n", argv[0]);
		break;
	case BMFS_CMD_CP:
		printf("%s cp SOURCE DESTINATION\n", argv[0]);
		break;
	case BMFS_CMD_FORMAT:
		printf("%s format [options]\n", argv0);
		printf("\n");
		printf("Options\n");
		printf("\t-f, --force     : Format an existing file system.\n");
		printf("\t-s, --size SIZE : Specify the size of the file system.\n");
		break;
	case BMFS_CMD_MKDIR:
		printf("%s mkdir PATH\n", argv0);
		break;
	case BMFS_CMD_LS:
		printf("%s ls [options] PATH\n", argv0);
		printf("\n");
		printf("Options:\n");
		printf("\t-c, --color MODE : Specify coloring mode ('always', 'never', 'auto').\n");
		printf("\t-l, --long-list  : Use long listing format.\n");
		break;
	case BMFS_CMD_TOUCH:
		printf("%s touch PATH\n", argv0);
		break;
	default:
		printf("No help available for '%s'\n", argv[0]);
		break;
	}
}

static void print_version(void)
{
	printf("BareMetal File System Utility v2.0.0 (2018 3 24)\n");
}
