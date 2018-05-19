#!/bin/bash

source ../bash/common.sh

set -e

CFLAGS="${CFLAGS} -fno-stack-protector"
CFLAGS="${CFLAGS} -fomit-frame-pointer"
CFLAGS="${CFLAGS} -mno-red-zone"

compile_file "crc32.c"
compile_file "dir.c"
compile_file "disk.c"
compile_file "encoding.c"
compile_file "entry.c"
compile_file "errno.c"
compile_file "file.c"
compile_file "fs.c"
compile_file "header.c"
compile_file "host.c"
compile_file "memcpy.c"
compile_file "path.c"
compile_file "status.c"
compile_file "table.c"

link_static "libbmfs.a" \
	"crc32.o"     \
	"dir.o "      \
	"disk.o "     \
	"encoding.o " \
	"entry.o "    \
	"errno.o "    \
	"file.o "     \
	"fs.o "       \
	"header.o "   \
	"host.o"      \
	"memcpy.o"    \
	"path.o "     \
	"status.o"    \
	"table.o"

compile_file "filedisk.c"
compile_file "ramdisk.c"
compile_file "size.c"
compile_file "stdhost.c"
compile_file "time.c"

link_static "libbmfs-stdlib.a" \
	"filedisk.o" \
	"ramdisk.o"  \
	"size.o"     \
	"stdhost.o"  \
	"time.o"
