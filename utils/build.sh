#!/bin/sh

set -e

CC=gcc
CFLAGS="${CFLAGS} -Wall -Wextra -Werror -Wfatal-errors -std=gnu99 -g"
CFLAGS="${CFLAGS} -I../include"

AR=ar
ARFLAGS=rcs

LD=gcc
LDFLAGS=-lc

$CC $CFLAGS -c bmfs.c

$LD $LDFLAGS bmfs.o ../lib/libbmfs.a ../lib/libbmfs-stdlib.a -o bmfs
