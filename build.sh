#!/bin/sh

set -e

cd lib
./build.sh
cd ..

cd utils
./build.sh
cd ..
