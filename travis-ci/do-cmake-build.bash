#!/bin/bash

set -e
set -u

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBMFS_WITH_POSIX_UTILS=ON -DBMFS_WITH_FUSE=ON
cmake --build .
ctest --verbose --output-on-failure
cpack
cmake --build . --target clean
