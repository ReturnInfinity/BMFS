#!/bin/sh

set -e
set -u

mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib
mkdir -p $PREFIX/include

cp utils/bmfs $PREFIX/bin
cp lib/libbmfs.a $PREFIX/lib/
cp -R include/bmfs $PREFIX/include/
