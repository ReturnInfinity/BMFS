#!/bin/sh

set -e

CC=gcc
CFLAGS="${CFLAGS} -Wall -Wextra -Werror -Wfatal-errors -std=gnu99 -g"
CFLAGS="${CFLAGS} -fno-stack-protector -fomit-frame-pointer -mno-red-zone"
CFLAGS="${CFLAGS} -I../include"

AR=ar
ARFLAGS=rcs

LD=gcc
LDFLAGS=-lc

$CC $CFLAGS -c header.c -o header.o
$CC $CFLAGS -c entry.c -o entry.o
$CC $CFLAGS -c dir.c -o dir.o
$CC $CFLAGS -c disk.c -o disk.o
$CC $CFLAGS -c ramdisk.c -o ramdisk.o

$AR $ARFLAGS libbmfs.a header.o entry.o dir.o disk.o ramdisk.o

$CC $CFLAGS -c stdlib.c -o stdlib.o
$CC $CFLAGS -c bmfs.c -o bmfs.o

$LD $LDFLAGS bmfs.o stdlib.o libbmfs.a -o bmfs
