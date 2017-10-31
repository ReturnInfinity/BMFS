#!/bin/bash

set -e
set -u

MAKEFLAGS="NO_FUSE= NO_UNIX_UTILS="
make $MAKEFLAGS
make test $MAKEFLAGS
make clean $MAKEFLAGS
