#!/bin/sh

set -e

CC=gcc
CFLAGS="${CFLAGS} -Wall -Wextra -Werror -Wfatal-errors -std=gnu99 -g"
CFLAGS="${CFLAGS} -fno-stack-protector"
CFLAGS="${CFLAGS} -fomit-frame-pointer"
CFLAGS="${CFLAGS} -mno-red-zone"
CFLAGS="${CFLAGS} -I../include"

AR=ar
ARFLAGS=rcs

$CC $CFLAGS -c dir.c
$CC $CFLAGS -c disk.c
$CC $CFLAGS -c encoding.c
$CC $CFLAGS -c entry.c
$CC $CFLAGS -c errno.c
$CC $CFLAGS -c file.c
$CC $CFLAGS -c fs.c
$CC $CFLAGS -c header.c
$CC $CFLAGS -c path.c
$CC $CFLAGS -c table.c

$AR $ARFLAGS libbmfs.a dir.o disk.o encoding.o entry.o errno.o file.o fs.o header.o path.o table.o

$CC $CFLAGS -c filedisk.c
$CC $CFLAGS -c ramdisk.c
$CC $CFLAGS -c size.c
$CC $CFLAGS -c time.c

$AR $ARFLAGS libbmfs-stdlib.a filedisk.o ramdisk.o size.o time.o
