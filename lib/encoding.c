/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#include <bmfs/encoding.h>

void bmfs_encode_uint64(bmfs_uint64 n, void *buf)
{
	unsigned char *buf8 = (unsigned char *) buf;

	buf8[0] = 0xff & (n >> 0x00);
	buf8[1] = 0xff & (n >> 0x08);
	buf8[2] = 0xff & (n >> 0x10);
	buf8[3] = 0xff & (n >> 0x18);
	buf8[4] = 0xff & (n >> 0x20);
	buf8[5] = 0xff & (n >> 0x28);
	buf8[6] = 0xff & (n >> 0x30);
	buf8[7] = 0xff & (n >> 0x38);
}

void bmfs_encode_uint32(bmfs_uint32 n, void *buf)
{
	unsigned char *buf8 = (unsigned char *) buf;

	buf8[0] = 0xff & (n >> 0x00);
	buf8[1] = 0xff & (n >> 0x08);
	buf8[2] = 0xff & (n >> 0x10);
	buf8[3] = 0xff & (n >> 0x18);
}

bmfs_uint64 bmfs_decode_uint64(const void *buf)
{
	const unsigned char *buf8 = (const unsigned char *) buf;

	bmfs_uint64 n = 0;

	n |= ((bmfs_uint64) buf8[0]) << 0x00;
	n |= ((bmfs_uint64) buf8[1]) << 0x08;
	n |= ((bmfs_uint64) buf8[2]) << 0x10;
	n |= ((bmfs_uint64) buf8[3]) << 0x18;
	n |= ((bmfs_uint64) buf8[4]) << 0x20;
	n |= ((bmfs_uint64) buf8[5]) << 0x28;
	n |= ((bmfs_uint64) buf8[6]) << 0x30;
	n |= ((bmfs_uint64) buf8[7]) << 0x38;

	return n;
}

bmfs_uint32 bmfs_decode_uint32(const void *buf)
{
	const unsigned char *buf8 = (const unsigned char *) buf;

	bmfs_uint64 n = 0;

	n |= ((bmfs_uint64) buf8[0]) << 0x00;
	n |= ((bmfs_uint64) buf8[1]) << 0x08;
	n |= ((bmfs_uint64) buf8[2]) << 0x10;
	n |= ((bmfs_uint64) buf8[3]) << 0x18;

	return n;
}
