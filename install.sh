#!/bin/sh

set -e
set -u

mkdir -p $PREFIX/include
mkdir -p $PREFIX/lib

cp src/libbmfs.a $PREFIX/lib/

cp -R include/bmfs $PREFIX/include/
