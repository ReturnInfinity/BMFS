#!/bin/sh

set -e

CC=gcc
CFLAGS="${CFLAGS} -Wall -Wextra -Werror -Wfatal-errors -std=gnu99 -g"
CFLAGS="${CFLAGS} -I../include"

AR=ar
ARFLAGS=rcs

$CC $CFLAGS -c dir.c
$CC $CFLAGS -c disk.c
$CC $CFLAGS -c entry.c
$CC $CFLAGS -c file.c
$CC $CFLAGS -c fs.c
$CC $CFLAGS -c header.c
$CC $CFLAGS -c path.c
$CC $CFLAGS -c table.c

$AR $ARFLAGS libbmfs.a dir.o disk.o entry.o file.o fs.o header.o path.o table.o

$CC $CFLAGS -c stdlib.c
$CC $CFLAGS -c sspec.c
$CC $CFLAGS -c ramdisk.c
$CC $CFLAGS -c time.c

$AR $ARFLAGS libbmfs-stdlib.a stdlib.o sspec.o ramdisk.o time.o
