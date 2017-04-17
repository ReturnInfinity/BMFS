#ifndef BMFS_SSPEC_H
#define BMFS_SSPEC_H

#include <stdint.h>

enum bmfs_sspec_type
{
	BMFS_SSPEC_NONE,
	/* 1024 ^ 4 */
	BMFS_SSPEC_TEBI,
	/* 1024 ^ 3 */
	BMFS_SSPEC_GIBI,
	/* 1024 ^ 2 */
	BMFS_SSPEC_MEBI,
	/* 1024 */
	BMFS_SSPEC_KIBI,
	/* 1000 ^ 4 */
	BMFS_SSPEC_TERA,
	/* 1000 ^ 3 */
	BMFS_SSPEC_GIGA,
	/* 1000 ^ 2 */
	BMFS_SSPEC_MEGA,
	/* 1000 */
	BMFS_SSPEC_KILO
};

struct bmfs_sspec
{
	enum bmfs_sspec_type type;
	uint64_t value;
};

int bmfs_sspec_parse(struct bmfs_sspec *sspec, const char *arg);

char * bmfs_sspec_to_string(const struct bmfs_sspec *spsec);

int bmfs_sspec_set_bytes(struct bmfs_sspec *sspec, uint64_t bytes);

int bmfs_sspec_bytes(const struct bmfs_sspec *sspec, uint64_t *bytes);

int bmfs_sspec_mebibytes(const struct bmfs_sspec *sspec, uint64_t *mebibytes);

#endif /* BMFS_SSPEC_H */

