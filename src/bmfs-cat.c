#include "bmfs.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void help(const char *argv0)
{
	printf("usage: %s [options]\n", argv0);
	printf("\n");
	printf("options:\n");
	printf("  --disk,      -d : specify disk image to use\n");
	printf("  --help,      -h : display this help message\n");
	printf("  --version,   -v : display version information\n");
	printf("\n");
	printf("environment variables:\n");
	printf("    BMFS_DISK : the disk image to use\n");
}

static void version(void)
{
	printf("%s\n", BMFS_VERSION_STRING);
}

static int cat_file(struct BMFSDisk *disk, const char *filename)
{
	struct BMFSEntry entry;

	int err = bmfs_disk_find_file(disk, filename, &entry, NULL);
	if (err != 0)
		return err;

	uint64_t entry_offset;
	err = bmfs_entry_get_offset(&entry, &entry_offset);
	if (err != 0)
		return err;

	err = bmfs_disk_seek(disk, entry_offset, SEEK_SET);
	if (err != 0)
		return err;

	char buf[32];

	uint64_t i = 0;
	while (i < entry.FileSize)
	{
		uint64_t read_count = 0;
		err = bmfs_disk_read(disk, buf, sizeof(buf), &read_count);
		if (err != 0)
			return err;

		fwrite(buf, read_count, 1, stdout);

		i += read_count;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct option opts[] =
	{
		{ "disk", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	const char *diskname = NULL;

	while (1)
	{
		int c = getopt_long(argc, argv, "d:n:r:hv", opts, NULL);
		if (c == 'd')
			diskname = optarg;
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

	if (optind >= argc)
	{
		fprintf(stderr, "%s: no files specified\n", argv[0]);
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
	int err = bmfs_disk_init_file(&disk, diskfile);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to initialize disk structure: %s\n", argv[0], strerror(-err));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	while (optind < argc)
	{
		err = cat_file(&disk, argv[optind]);
		if (err != 0)
		{
			fprintf(stderr, "%s: failed to cat '%s': %s\n", argv[0], argv[optind], strerror(-err));
			fclose(diskfile);
			return EXIT_FAILURE;
		}
		optind++;
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

