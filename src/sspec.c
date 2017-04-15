#include "sspec.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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

	if (str[0] == 0)
		sspec->type = BMFS_SSPEC_NONE;
	else if (strcmp(str, "B") == 0)
		sspec->type = BMFS_SSPEC_NONE;
	else if (strcmp(str, "TiB") == 0)
		sspec->type = BMFS_SSPEC_TEBI;
	else if (strcmp(str, "TB") == 0)
		sspec->type = BMFS_SSPEC_TERA;
	else if (strcmp(str, "GiB") == 0)
		sspec->type = BMFS_SSPEC_GIBI;
	else if (strcmp(str, "GB") == 0)
		sspec->type = BMFS_SSPEC_GIGA;
	else if (strcmp(str, "MiB") == 0)
		sspec->type = BMFS_SSPEC_MEBI;
	else if (strcmp(str, "MB") == 0)
		sspec->type = BMFS_SSPEC_MEGA;
	else if (strcmp(str, "KiB") == 0)
		sspec->type = BMFS_SSPEC_KIBI;
	else if (strcmp(str, "KB") == 0)
		sspec->type = BMFS_SSPEC_KILO;
	else
		return -EINVAL;

	sspec->value = value;

	return 0;
}

char * bmfs_sspec_to_string(const struct bmfs_sspec *sspec)
{
	uint64_t bytes = 0;

	int err = bmfs_sspec_bytes(sspec, &bytes);
	if (err != 0)
		return NULL;

	size_t str_len = 32;
	/* calloc instead of malloc,
	 * just for safe measure */
	char *str = calloc(1, str_len);
	if (str == NULL)
		return NULL;

	uint64_t value = sspec->value;
	if (value >= 1024ULL * 1024ULL * 1024ULL * 1024ULL)
	{
		value /= 1024ULL * 1024ULL * 1024ULL * 1024ULL;
		snprintf(str, str_len, "%" PRIu64 "TiB", value);
	}
	else if (value >= 1024ULL * 1024ULL * 1024ULL)
	{
		value /= 1024ULL * 1024ULL * 1024ULL;
		snprintf(str, str_len, "%" PRIu64 "GiB", value);
	}
	else if (value >= 1024ULL * 1024ULL)
	{
		value /= 1024ULL * 1024ULL;
		snprintf(str, str_len, "%" PRIu64 "MiB", value);
	}
	else if (value >= 1024ULL)
	{
		value /= 1024ULL;
		snprintf(str, str_len, "%" PRIu64 "KiB", value);
	}
	else
		snprintf(str, str_len, "%" PRIu64 "B", value);

	return str;
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

