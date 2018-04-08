/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/encoding.h>

#include "assert.h"

#include <stdlib.h>
#include <string.h>

static bmfs_bool test_encoding64(bmfs_uint64 n, const void *buf)
{
	unsigned char out[8];

	bmfs_encode_uint64(n, out);

	if (memcmp(out, buf, 8) == 0)
		return BMFS_TRUE;
	else
		return BMFS_FALSE;
}

static bmfs_bool test_encoding32(bmfs_uint32 n, const void *buf)
{
	unsigned char out[4];

	bmfs_encode_uint32(n, out);

	if (memcmp(out, buf, 4) == 0)
		return BMFS_TRUE;
	else
		return BMFS_FALSE;
}

int main(void)
{
	bmfs_assert(test_encoding64(0x1122334455667788, "\x88\x77\x66\x55\x44\x33\x22\x11"));
	bmfs_assert(test_encoding32(0xaabbccdd, "\xdd\xcc\xbb\xaa"));

	return EXIT_SUCCESS;
}
