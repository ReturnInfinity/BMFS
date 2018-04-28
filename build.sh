#!/bin/bash

set -e

function build_dir {
	echo "Entering ${PWD}/$1"
	cd $1
	./build.sh
	cd ..
}

build_dir "lib"
build_dir "utils"
