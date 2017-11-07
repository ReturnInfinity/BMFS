#!/bin/sh

set -e
set -u

mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib
mkdir -p $PREFIX/include

cp src/bmfs $PREFIX/bin
cp src/libbmfs.a $PREFIX/lib/
cp -R include/bmfs $PREFIX/include/
