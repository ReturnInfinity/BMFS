#include "bmfs.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void help(const char *argv0)
{
	printf("usage: %s [options] file...\n", argv0);
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
		printf("%s: no files specified\n", argv[0]);
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

	while (optind < argc)
	{
		const char *filename = argv[optind];

		err = bmfs_disk_create_file(&disk, filename, reserved_mebibytes);
		if (err != 0)
		{
			fprintf(stderr, "%s: failed to create '%s': %s\n", argv[0], filename, strerror(-err));
			fclose(diskfile);
			return EXIT_FAILURE;
		}

		optind++;
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

