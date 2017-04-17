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
	printf("Removes an entry from a BMFS formatted file or drive.\n");
	printf("\n");
	printf("options:\n");
	printf("  --disk,    -d : specify disk image to use\n");
	printf("  --force,   -f : ignore non-existant files\n");
	printf("  --help,    -h : display this help message\n");
	printf("  --version, -v : display version information\n");
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
	int force_flag = 0;

	struct option opts[] =
	{
		{ "disk", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "force", no_argument, &force_flag, 1 },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	const char *diskname = NULL;

	while (1)
	{
		int c = getopt_long(argc, argv, "d:n:r:fhv", opts, NULL);
		if (c == 'd')
			diskname = optarg;
		else if (c == 'f')
			force_flag = 1;
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
		const char *filename = argv[optind];

		err = bmfs_disk_delete_file(&disk, filename);
		if ((err != 0) && !(force_flag && (err == -ENOENT)))
		{
			fprintf(stderr, "%s: failed to delete '%s': %s\n", argv[0], filename, strerror(-err));
			fclose(diskfile);
			return EXIT_FAILURE;
		}

		optind++;
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

