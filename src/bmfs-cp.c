#include <bmfs/bmfs.h>
#include <bmfs/stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static volatile int keep_reading = 1;

void handle_interrupt(int sig)
{
	if (sig == SIGINT)
		keep_reading = 0;
}

/* Returns the basename of the path.
 * This function is similar to the
 * one in glibc, except that it doesn't
 * modify the argument.
 *
 * The trailing underscore was added
 * to avoid a name collision with said
 * glibc function.
 */

static const char * basename_(const char *src)
{
	size_t src_len = strlen(src);
	if (src_len == 0)
		return NULL;

	size_t src_pos = src_len;
	while (src_pos > 0)
	{
		char c = src[src_pos - 1];
		if ((c == '/') || (c == '\\'))
			break;
		src_pos--;
	}

	return &src[src_pos];
}

static int copy_file(struct BMFSDisk *disk, const char *src, const char *dst, uint64_t reserved_mebibytes)
{
	if (src == NULL)
		src = "-";

	if (dst == NULL)
	{
		/* must specify destination
		 * if stdin is being used */
		if (strcmp(src, "-") == 0)
			return -EINVAL;

		dst = basename_(src);
		if (dst == NULL)
			/* src was an empty string */
			return -EINVAL;
		else if (dst[0] == 0)
			/* src had a trailing slash */
			return -EISDIR;
	}

	struct BMFSDir dir;
	int err = bmfs_disk_read_dir(disk, &dir);
	if (err != 0)
		return err;

	struct BMFSEntry *entry;
	entry = bmfs_dir_find(&dir, dst);
	if (entry == NULL)
	{
		err = bmfs_disk_create_file(disk, dst, reserved_mebibytes);
		if (err != 0)
			return err;

		/* if a file was added to the
		 * root directory, we'll have
		 * to read the directory again
		 * to get the file's starting
		 * point. */

		err = bmfs_disk_read_dir(disk, &dir);
		if (err != 0)
			return err;

		entry = bmfs_dir_find(&dir, dst);
		if (entry == NULL)
			/* for some reason, the
			 * file creation failed
			 * and the entry doesn't
			 * exist. */
			return -ENOENT;
	}

	uint64_t entry_offset;
	err = bmfs_entry_get_offset(entry, &entry_offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, entry_offset, SEEK_SET);
	if (err != 0)
		return err;

	FILE *srcfile = stdin;
	if (strcmp(src, "-") != 0)
	{
		srcfile = fopen(src, "rb");
		if (srcfile == NULL)
			return -errno;
	}

	uint64_t entry_size = entry->ReservedBlocks * BMFS_BLOCK_SIZE;
	uint64_t i = 0;

	char buf[32];
	size_t buf_size = sizeof(buf);

	if (srcfile == stdin)
		/* only rely on reading one
		 * byte at a time if using
		 * standard input */
		buf_size = 1;

	while (!feof(srcfile) && keep_reading)
	{
		size_t read_count = fread(buf, 1, buf_size, srcfile);
		if ((i + read_count) > entry_size)
		{
			/* not enough blocks reserved for file */
			if (srcfile != stdin)
				fclose(srcfile);
			return -ENOSPC;
		}

		err = bmfs_disk_write(disk, buf, read_count, NULL);
		if (err != 0)
		{
			if (srcfile != stdin)
				fclose(srcfile);
			return err;
		}

		i += read_count;
	}

	if (srcfile != stdin)
		fclose(srcfile);

	entry->FileSize = i;

	err = bmfs_disk_write_dir(disk, &dir);
	if (err != 0)
		return err;

	return 0;
}

static void help(const char *argv0)
{
	printf("usage: %s [options] src [dst]\n", argv0);
	printf("\n");
	printf("Copies a file from the host system to the BMFS\n");
	printf("formatted file system. If no destination is given,\n");
	printf("the destination file will be named using the basename\n");
	printf("of the source argument.\n");
	printf("\n");
	printf("options:\n");
	printf("  --disk, -d             : specify disk image to use\n");
	printf("  --help, -h             : display this help message\n");
	printf("  --reserved-storage, -r : the number of bytes to reserve for the file\n");
	printf("  --version, -v          : display version information\n");
	printf("\n");
	printf("environment variables:\n");
	printf("    BMFS_DISK : the disk image to use\n");
}

static void version(void)
{
	printf("%s\n", BMFS_VERSION_STRING);
}

int main(int argc, char **argv)
{
	signal(SIGINT, handle_interrupt);

	struct option opts[] =
	{
		{ "disk", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "reserved-storage", required_argument, NULL, 'r' },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	const char *diskname = NULL;
	struct bmfs_sspec reserved_storage;

	int err = bmfs_sspec_parse(&reserved_storage, "2MiB");
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to parse default file size\n", argv[0]);
		return EXIT_FAILURE;
	}

	while (1)
	{
		int c = getopt_long(argc, argv, "d:n:r:hv", opts, NULL);
		if (c == 'd')
			diskname = optarg;
		if (c == 'r')
		{
			err = bmfs_sspec_parse(&reserved_storage, optarg);
			if (err != 0)
			{
				fprintf(stderr, "%s: failed to parse size '%s': %s\n", argv[0], optarg, strerror(-err));
				return EXIT_FAILURE;
			}
		}
		else if (c == 'h')
		{
			help(argv[0]);
			return EXIT_FAILURE;
		}
		else if (c == 'v')
		{
			version();
			return EXIT_FAILURE;
		}
		else if (c == -1)
			/* end of options */
			break;
		else if (c == ':')
			/* invalid option */
			return EXIT_FAILURE;
		else if (c == '?')
			/* missing option argument */
			return EXIT_FAILURE;
	}

	if (diskname == NULL)
	{
		diskname = getenv("BMFS_DISK");
		if (diskname == NULL)
			diskname = "disk.image";
	}

	/* check to make sure there are
	 * files before opening the disk */
	if (optind >= argc)
	{
		printf("%s: no file specified\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char *src = argv[optind];

	const char *dst = NULL;
	if ((optind + 1) < argc)
		dst = argv[optind + 1];

	if ((optind + 2) < argc)
	{
		printf("%s: trailing argument '%s'\n", argv[0], argv[optind]);
		return EXIT_FAILURE;
	}

	/* make sure that the reserved storage
	 * is at least one block size */
	uint64_t reserved_bytes;
	err = bmfs_sspec_bytes(&reserved_storage, &reserved_bytes);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to calculate reserved MiB: %s\n", argv[0], strerror(-err));
		return EXIT_FAILURE;
	}
	else if (reserved_bytes < BMFS_BLOCK_SIZE)
	{
		fprintf(stderr, "%s: reserved storage must be at least one block size (%lluB)\n", argv[0], BMFS_BLOCK_SIZE);
		return EXIT_FAILURE;
	}

	/* calculate the number of mebibytes
	 * each file will have */
	uint64_t reserved_mebibytes;
	err = bmfs_sspec_mebibytes(&reserved_storage, &reserved_mebibytes);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to calculate reserved MiB: %s\n", argv[0], strerror(-err));
		return EXIT_FAILURE;
	}

	FILE *diskfile;
	diskfile = fopen(diskname, "r+b");
	if (diskfile == NULL)
	{
		fprintf(stderr, "%s: failed to open '%s': %s\n", argv[0], diskname, strerror(errno));
		return EXIT_FAILURE;
	}

	struct BMFSDisk disk;
	err = bmfs_disk_init_file(&disk, diskfile);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to initialize disk structure: %s\n", argv[0], strerror(-err));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	err = copy_file(&disk, src, dst, reserved_mebibytes);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to copy '%s': %s\n", argv[0], src, strerror(-err));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

