/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/size.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef _MSC_VER
#pragma warning(disable : 4244)
#endif /* _MSC_VER */

static int to_string(uint64_t bytes, char *str, uint64_t str_len);

static int to_type(const char *suffix, enum BMFSSizeSuffix *type);

int bmfs_size_parse(struct BMFSSize *size, const char *str)
{
	if ((size == NULL) || (str == NULL))
		return -EFAULT;

	uint64_t value = 0;

	uint64_t base = 1;

	while (*str)
	{
		char c = *str;
		if ((c < '0') || (c > '9'))
			break;

		value *= base;

		value += (c - '0');

		base *= 10;

		str++;
	}

	int err = to_type(str, &size->Suffix);
	if (err != 0)
		return err;

	size->Value = value;

	return 0;
}

int bmfs_size_to_string(const struct BMFSSize *size, char *str, uint64_t str_len)
{
	uint64_t bytes = 0;

	int err = bmfs_size_bytes(size, &bytes);
	if (err != 0)
		return err;

	return to_string(bytes, str, str_len);
}

int bmfs_size_set_bytes(struct BMFSSize *size, uint64_t bytes)
{
	size->Suffix = BMFS_SIZE_SUFFIX_NONE;
	size->Value = bytes;
	return 0;
}

int bmfs_size_bytes(const struct BMFSSize *size, uint64_t *bytes)
{
	if ((size == NULL) || (bytes == NULL))
		return -EFAULT;

	if (size->Suffix == BMFS_SIZE_SUFFIX_TEBI)
		*bytes = size->Value * 1024ULL * 1024ULL * 1024ULL * 1024ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_TERA)
		*bytes = size->Value * 1000ULL * 1000ULL * 1000ULL * 1000ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_GIBI)
		*bytes = size->Value * 1024ULL * 1024ULL * 1024ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_GIGA)
		*bytes = size->Value * 1000ULL * 1000ULL * 1000ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_MEBI)
		*bytes = size->Value * 1024ULL * 1024ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_MEGA)
		*bytes = size->Value * 1000ULL * 1000ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_KIBI)
		*bytes = size->Value * 1024ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_KILO)
		*bytes = size->Value * 1000ULL;
	else if (size->Suffix == BMFS_SIZE_SUFFIX_NONE)
		*bytes = size->Value;

	return 0;
}

int bmfs_size_mebibytes(const struct BMFSSize *size, uint64_t *mebibytes)
{
	if ((size == NULL)
	 || (mebibytes == NULL))
		return -EFAULT;

	int err = bmfs_size_bytes(size, mebibytes);
	if (err != 0)
		return err;

	*mebibytes /= 1024ULL * 1024ULL;

	return 0;
}

static int to_string(uint64_t bytes, char *str, uint64_t str_len)
{
	uint64_t base = 0;
	uint64_t i = 0;
	uint64_t n = 0;
	const char *suffix = NULL;

	/* max string: "1024YiB" plus '\0' = 8 */
	if (str_len < 8)
		return -EINVAL;

	while (bytes > 1024)
	{
		bytes /= 1024;
		base++;
	}

	if (base == 0)
		suffix = "B";
	else if (base == 1)
		suffix = "KiB";
	else if (base == 2)
		suffix = "MiB";
	else if (base == 3)
		suffix = "GiB";
	else if (base == 4)
		suffix = "TiB";
	else if (base == 5)
		suffix = "PiB";
	else if (base == 6)
		suffix = "EiB";
	else if (base == 7)
		suffix = "ZiB";
	else if (base == 8)
		suffix = "YiB";
	else
		return -EINVAL;

	if (bytes == 0)
	{
		str[i] = '0';
		i++;
	}

	n = bytes / 1000;
	if (n > 0)
	{
		str[i] = '0' + n;
		i++;
		bytes %= 1000;
	}

	n = bytes / 100;
	if (n > 0)
	{
		str[i] = '0' + n;
		i++;
		bytes %= 100;
	}

	n = bytes / 10;
	if (n > 0)
	{
		str[i] = '0' + n;
		i++;
		bytes %= 10;
	}

	n = bytes;
	str[i] = '0' + n;
	i++;

	while (*suffix)
	{
		str[i] = *suffix;
		suffix++;
		i++;
	}

	str[i] = 0;

	return 0;
}

static int to_type(const char *suffix, enum BMFSSizeSuffix *type)
{
	if (suffix[0] == 0)
	{
		*type = BMFS_SIZE_SUFFIX_NONE;
		return 0;
	}

	if ((suffix[0] == 'B')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_NONE;
		return 0;
	}

	if ((suffix[0] == 'K')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_KILO;
		return 0;
	}

	if ((suffix[0] == 'M')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_MEGA;
		return 0;
	}

	if ((suffix[0] == 'G')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_GIGA;
		return 0;
	}

	if ((suffix[0] == 'T')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_TERA;
		return 0;
	}

	if ((suffix[0] == 'K')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_KIBI;
		return 0;
	}

	if ((suffix[0] == 'M')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_MEBI;
		return 0;
	}

	if ((suffix[0] == 'G')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_GIBI;
		return 0;
	}

	if ((suffix[0] == 'T')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_TEBI;
		return 0;
	}

	if ((suffix[0] == 'K')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_KIBI;
		return 0;
	}

	if ((suffix[0] == 'M')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_MEBI;
		return 0;
	}

	if ((suffix[0] == 'G')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_GIBI;
		return 0;
	}

	if ((suffix[0] == 'T')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SIZE_SUFFIX_TEBI;
		return 0;
	}

	return -EINVAL;
}

