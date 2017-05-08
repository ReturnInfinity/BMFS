#include <bmfs/sspec.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

static int to_string(uint64_t bytes, char *str, uint64_t str_len);

static int to_type(const char *suffix, enum bmfs_sspec_type *type);

int bmfs_sspec_parse(struct bmfs_sspec *sspec, const char *str)
{
	if ((sspec == NULL) || (str == NULL))
		return -EFAULT;

	uint64_t value = 0;
	uint64_t base = 1;
	while (*str)
	{
		char c = *str;
		if ((c < '0')
		 || (c > '9'))
			break;
		value += (c - '0') * base;
		base *= 10;
		str++;
	}

	int err = to_type(str, &sspec->type);
	if (err != 0)
		return err;

	sspec->value = value;

	return 0;
}

int bmfs_sspec_to_string(const struct bmfs_sspec *sspec, char *str, uint64_t str_len)
{
	uint64_t bytes = 0;

	int err = bmfs_sspec_bytes(sspec, &bytes);
	if (err != 0)
		return err;

	return to_string(bytes, str, str_len);
}

int bmfs_sspec_set_bytes(struct bmfs_sspec *sspec, uint64_t bytes)
{
	sspec->type = BMFS_SSPEC_NONE;
	sspec->value = bytes;
	return 0;
}

int bmfs_sspec_bytes(const struct bmfs_sspec *sspec, uint64_t *bytes)
{
	if ((sspec == NULL) || (bytes == NULL))
		return -EFAULT;

	if (sspec->type == BMFS_SSPEC_TEBI)
		*bytes = sspec->value * 1024ULL * 1024ULL * 1024ULL * 1024ULL;
	else if (sspec->type == BMFS_SSPEC_TERA)
		*bytes = sspec->value * 1000ULL * 1000ULL * 1000ULL * 1000ULL;
	else if (sspec->type == BMFS_SSPEC_GIBI)
		*bytes = sspec->value * 1024ULL * 1024ULL * 1024ULL;
	else if (sspec->type == BMFS_SSPEC_GIGA)
		*bytes = sspec->value * 1000ULL * 1000ULL * 1000ULL;
	else if (sspec->type == BMFS_SSPEC_MEBI)
		*bytes = sspec->value * 1024ULL * 1024ULL;
	else if (sspec->type == BMFS_SSPEC_MEGA)
		*bytes = sspec->value * 1000ULL * 1000ULL;
	else if (sspec->type == BMFS_SSPEC_KIBI)
		*bytes = sspec->value * 1024ULL;
	else if (sspec->type == BMFS_SSPEC_KILO)
		*bytes = sspec->value * 1000ULL;
	else if (sspec->type == BMFS_SSPEC_NONE)
		*bytes = sspec->value;

	return 0;
}

int bmfs_sspec_mebibytes(const struct bmfs_sspec *sspec, uint64_t *mebibytes)
{
	if ((sspec == NULL)
	 || (mebibytes == NULL))
		return -EFAULT;

	int err = bmfs_sspec_bytes(sspec, mebibytes);
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

static int to_type(const char *suffix, enum bmfs_sspec_type *type)
{
	if (suffix[0] == 0)
	{
		*type = BMFS_SSPEC_NONE;
		return 0;
	}

	if ((suffix[0] == 'B')
	 && (suffix[1] == 0))
	{
		*type = BMFS_SSPEC_NONE;
		return 0;
	}

	if ((suffix[0] == 'K')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SSPEC_KILO;
		return 0;
	}

	if ((suffix[0] == 'M')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SSPEC_MEGA;
		return 0;
	}

	if ((suffix[0] == 'G')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SSPEC_GIGA;
		return 0;
	}

	if ((suffix[0] == 'T')
	 && (suffix[1] == 'B')
	 && (suffix[2] == 0))
	{
		*type = BMFS_SSPEC_TERA;
		return 0;
	}

	if ((suffix[0] == 'K')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SSPEC_KIBI;
		return 0;
	}

	if ((suffix[0] == 'M')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SSPEC_MEBI;
		return 0;
	}

	if ((suffix[0] == 'G')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SSPEC_GIBI;
		return 0;
	}

	if ((suffix[0] == 'T')
	 && (suffix[1] == 'i')
	 && (suffix[2] == 'B')
	 && (suffix[3] == 0))
	{
		*type = BMFS_SSPEC_TEBI;
		return 0;
	}

	return -EINVAL;
}

