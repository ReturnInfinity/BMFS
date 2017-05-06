#ifndef BMFS_SSPEC_H
#define BMFS_SSPEC_H

#include <stdint.h>

/** @file */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup sspec-api Storage Specifiers
 * Read and write values like 'MiB', 'GB' and others.
 */

/** Specifies which suffix is used
 * in the storage specification.
 * @ingroup sspec-api
 */

enum bmfs_sspec_type
{
	BMFS_SSPEC_NONE,
	/** equivalent to 1024 ^ 4 */
	BMFS_SSPEC_TEBI,
	/** equivalent to 1024 ^ 3 */
	BMFS_SSPEC_GIBI,
	/** equivalent to 1024 ^ 2 */
	BMFS_SSPEC_MEBI,
	/** equivalent to 1024 */
	BMFS_SSPEC_KIBI,
	/** equivalent to 1000 ^ 4 */
	BMFS_SSPEC_TERA,
	/** equivalent to 1000 ^ 3 */
	BMFS_SSPEC_GIGA,
	/** equivalent to 1000 ^ 2 */
	BMFS_SSPEC_MEGA,
	/** equivalent to 1000 */
	BMFS_SSPEC_KILO
};

/** Used to specify a numerical value
 * of storage capacity. It is an abbreviated
 * name of 'storage specification'.
 * @ingroup sspec-api
 */

struct bmfs_sspec
{
	/** The type of storage specification */
	enum bmfs_sspec_type type;
	/** The number of storage units of the
	 * specifications type. For example: a
	 * value of two and a type of Mebibytes,
	 * would be equivalent to 2MiB. */
	uint64_t value;
};

/** Parses a string describing a storage capacity.
 * A couple of examples of strings would be:
 *  - 3KB
 *  - 9B
 *  - 7 (equivalent to 7B)
 *  - 0
 * @param sspec A storage specification structure.
 *  This parameter is initialized by the function.
 * @param arg A string representation of a storage
 *  capacity. The string must contain no spaces and
 *  is case-sensitive.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup sspec-api
 */

int bmfs_sspec_parse(struct bmfs_sspec *sspec, const char *arg);

/** Converts a storage specification structure
 * to a human-reabable string.
 * @param sspec An initialized storage specification.
 * @param str A string to write the storage value to.
 * @param str_len The number of bytes available for @p str.
 *  This value must be at least eight.
 * @returns Zero on success, a negative error code on
 *  failure.
 * @ingroup sspec-api
 */

int bmfs_sspec_to_string(const struct bmfs_sspec *sspec, char *str, uint64_t str_len);

/** Sets the value, in bytes, of the storage specification.
 * @param sspec A storage specifier structure. This parameter
 *  is set by the function.
 * @param bytes The value to set the storage specifier to.
 * @returns Zero on success, a negative error code on failure.
 * @ingroup sspec-api
 */

int bmfs_sspec_set_bytes(struct bmfs_sspec *sspec, uint64_t bytes);

/** Retrives the number of bytes of the storage specification.
 * This function may fail if the storage specification is not
 * able to be represented by a 64-bit unsigned integer.
 * @param sspec An initialized storage specification.
 * @param bytes The address of the variable that will receive
 *  the number of bytes of the storage specifier.
 * @returns Zero on success, a negative error on failure.
 * @ingroup sspec-api
 */

int bmfs_sspec_bytes(const struct bmfs_sspec *sspec, uint64_t *bytes);

/** Retrives the number of mebibytes of the storage specification.
 * This function may fail if the storage specification is not
 * able to be represented by a 64-bit unsigned integer.
 * @param sspec An initialized storage specification.
 * @param mebibytes The address of the variable that will receive
 *  the number of mebibytes of the storage specifier.
 * @returns Zero on success, a negative error on failure.
 * @ingroup sspec-api
 */

int bmfs_sspec_mebibytes(const struct bmfs_sspec *sspec, uint64_t *mebibytes);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* BMFS_SSPEC_H */

