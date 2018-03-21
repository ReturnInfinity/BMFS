#include <bmfs/bmfs.h>
#include <bmfs/stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exists(const char * filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL)
		return 0;
	fclose(file);
	return 1;
}

static void help(const char *argv0)
{
	printf("usage: %s [options] file\n", argv0);
	printf("\n");
	printf("Formats a file with BMFS.\n");
	printf("\n");
	printf("options:\n");
	printf("  --disk, -d      : specify disk image to use\n");
	printf("  --disk-size, -s : specify storage to allocate for disk\n");
	printf("  --force, -f     : format file, even if it already exists\n");
	printf("  --help, -h      : display this help message\n");
	printf("  --version, -v   : display version information\n");
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
		{ "disk-size", required_argument, NULL, 's' },
		{ "force", no_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	const char *diskname = NULL;
	struct bmfs_sspec disk_storage;

	int err = bmfs_sspec_parse(&disk_storage, "128MiB");
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to parse default file size\n", argv[0]);
		return EXIT_FAILURE;
	}

	while (1)
	{
		int c = getopt_long(argc, argv, "d:n:r:hfv", opts, NULL);
		if (c == 'd')
			diskname = optarg;
		else if (c == 'f')
			force_flag = 1;
		else if (c == 's')
		{
			err = bmfs_sspec_parse(&disk_storage, optarg);
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

	/* make sure that the disk storage
	 * is enough for the file system.
	 */
	uint64_t disk_size;
	err = bmfs_sspec_bytes(&disk_storage, &disk_size);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to calculate disk size: %s\n", argv[0], strerror(-err));
		return EXIT_FAILURE;
	}
	else if (disk_size < BMFS_MINIMUM_DISK_SIZE)
	{
		fprintf(stderr, "%s: disk size must be at least %lluB\n", argv[0], BMFS_MINIMUM_DISK_SIZE);
		return EXIT_FAILURE;
	}

	if (exists(diskname) && !force_flag)
	{
		fprintf(stderr, "%s: failed to format '%s': %s\n", argv[0], diskname, strerror(EEXIST));
		fprintf(stderr, "  use --force or -f to override\n");
		return EXIT_FAILURE;
	}

	FILE *diskfile;
	diskfile = fopen(diskname, "wb");
	if (diskfile == NULL)
	{
		fprintf(stderr, "%s: failed to open '%s': %s\n", argv[0], diskname, strerror(errno));
		return EXIT_FAILURE;
	}

	err = fseek(diskfile, disk_size - 1, SEEK_SET);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to seek '%s': %s\n", argv[0], diskname, strerror(errno));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	if (fputc(0, diskfile) != 0)
	{
		fprintf(stderr, "%s: failed to write terminating byte to '%s': %s\n", argv[0], diskname, strerror(errno));
		fclose(diskfile);
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

	err = bmfs_disk_format(&disk);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to format '%s': %s\n", argv[0], diskname, strerror(-err));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

