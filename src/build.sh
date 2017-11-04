#!/bin/sh

CC=gcc
CFLAGS="${CFLAGS} -Wall -Wextra -Werror -Wfatal-errors -std=gnu99"
CFLAGS="${CFLAGS} -fno-stack-protector -fomit-frame-pointer -mno-red-zone"
CFLAGS="${CFLAGS} -I../include"

LD=gcc
LDFLAGS=-lc

$CC $CFLAGS -c entry.c -o entry.o
$CC $CFLAGS -c dir.c -o dir.o
$CC $CFLAGS -c disk.c -o disk.o
$CC $CFLAGS -c bmfs.c -o bmfs.o
$CC $CFLAGS -c stdlib.c -o stdlib.o
$LD $LDFLAGS entry.o dir.o disk.o bmfs.o stdlib.o -o bmfs
