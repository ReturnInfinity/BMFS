#include "sspec.h"

#include <errno.h>
#include <stdlib.h>

int bmfs_sspec_parse(struct bmfs_sspec *sspec, const char *str)
{
	if ((sspec == NULL) || (str == NULL))
		return -EFAULT;

	return 0;
}

int bmfs_sspec_bytes(const struct bmfs_sspec *sspec, uint64_t *bytes)
{
	if ((sspec == NULL) || (bytes == NULL))
		return -EFAULT;

	return 0;
}

