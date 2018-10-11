#!/bin/bash

set -e
set -u

mkdir -p "${PREFIX}/bin"
mkdir -p "${PREFIX}/lib"
mkdir -p "${PREFIX}/include/bmfs"

function update_file {
	echo "Updating $2"
	cp "$1" "$2"
}

update_file "utils/bmfs" "${PREFIX}/bin/bmfs"
update_file "lib/libbmfs.a" "${PREFIX}/lib/libbmfs.a"
update_file "lib/libbmfs-stdlib.a" "${PREFIX}/lib/libbmfs-stdlib.a"

for header in include/bmfs/*.h; do
	update_file "$header" "${PREFIX}/$header"
done
