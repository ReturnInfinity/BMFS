#!/bin/bash

set -e
set -u

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBMFS_WITH_FUSE=ON
cmake --build .
ctest --verbose --output-on-failure
cpack -G DEB -D CPACK_PACKAGING_INSTALL_PREFIX=/opt/return-infinity
cmake --build . --target clean
