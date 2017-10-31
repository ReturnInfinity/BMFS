#include <bmfs/bmfs.h>
#include <bmfs/stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void help(const char *argv0)
{
	printf("usage: %s [options]\n", argv0);
	printf("\n");
	printf("Lists entries in a BMFS formatted file or disk.\n");
	printf("\n");
	printf("options:\n");
	printf("  --disk,      -d : specify disk image to use\n");
	printf("  --help,      -h : display this help message\n");
	printf("               -l : show file size and reserved size\n");
	printf("  --show-size     : show the file size (on|off)\n");
	printf("  --show-reserved : show the reserved size (on|off)\n");
	printf("  --version,   -v : display version information\n");
	printf("\n");
	printf("environment variables:\n");
	printf("    BMFS_DISK : the disk image to use\n");
}

static int parse_bool(const char *str)
{
	if (str == NULL)
		return -EFAULT;

	if ((strcmp(str, "yes") == 0)
	 || (strcmp(str, "on") == 0)
	 || (strcmp(str, "1") == 0))
		return 1;

	if ((strcmp(str, "no") == 0)
	 || (strcmp(str, "off") == 0)
	 || (strcmp(str, "0") == 0))
		return 0;

	/* value not understood */

	return -EINVAL;
}

static void version(void)
{
	printf("%s\n", BMFS_VERSION_STRING);
}

int main(int argc, char **argv)
{
	int show_size = 0;
	int show_reserved = 0;

	struct option opts[] =
	{
		{ "disk", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "show-size", required_argument, NULL, 's' },
		{ "show-reserved", required_argument, NULL, 'r' },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	const char *diskname = NULL;

	while (1)
	{
		int c = getopt_long(argc, argv, "d:n:r:hlv", opts, NULL);
		if (c == 'd')
			diskname = optarg;
		else if (c == 'h')
		{
			help(argv[0]);
			return EXIT_FAILURE;
		}
		else if (c == 'l')
		{
			show_size = 1;
			show_reserved = 1;
		}
		else if (c == 'r')
		{
			show_reserved = parse_bool(optarg);
			if (show_reserved < 0)
			{
				fprintf(stderr, "%s: unknown setting '%s'\n", argv[0], optarg);
				return EXIT_FAILURE;
			}
		}
		else if (c == 's')
		{
			show_size = parse_bool(optarg);
			if (show_size < 0)
			{
				fprintf(stderr, "%s: unknown setting '%s'\n", argv[0], optarg);
				return EXIT_FAILURE;
			}
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

	struct BMFSDir dir;
	err = bmfs_disk_read_root_dir(&disk, &dir);
	if (err != 0)
	{
		fprintf(stderr, "%s: failed to read root directory: %s\n", argv[0], strerror(-err));
		fclose(diskfile);
		return EXIT_FAILURE;
	}

	for (uint64_t i = 0ULL; i < 64ULL; i++)
	{
		const struct BMFSEntry *entry;
		entry = &dir.Entries[i];
		if (bmfs_entry_is_empty(entry))
			continue;
		else if (bmfs_entry_is_terminator(entry))
			break;

		if (show_size)
		{
			struct bmfs_sspec entry_storage;
			bmfs_sspec_set_bytes(&entry_storage, entry->FileSize);
			char entry_size[8];
			err = bmfs_sspec_to_string(&entry_storage, entry_size, sizeof(entry_size));
			if (err == 0)
				printf("%-7s ", entry_size);
		}

		if (show_reserved)
		{
			struct bmfs_sspec reserved_storage;
			bmfs_sspec_set_bytes(&reserved_storage, entry->ReservedBlocks * BMFS_BLOCK_SIZE);
			char entry_reserved[8];
			err = bmfs_sspec_to_string(&reserved_storage, entry_reserved, sizeof(entry_reserved));
			if (err == 0)
				printf("%-7s ", entry_reserved);
		}

		printf("%s\n", entry->FileName);
	}

	fclose(diskfile);

	return EXIT_SUCCESS;
}

