#!/bin/bash

set -e

source "../bash/common.sh"

compile_file "bmfs.c"

LDFLAGS="${LDFLAGS} -L ../lib"
LDLIBS="${LDLIBS} -lbmfs"
LDLIBS="${LDLIBS} -lbmfs-stdlib"

link_executable "bmfs" "bmfs.o"
