cmake_minimum_required(VERSION 2.6)

add_library("bmfs-library"
	"encoding.c"
	"entry.c"
	"errno.c"
	"dir.c"
	"disk.c"
	"file.c"
	"fs.c"
	"header.c"
	"path.c"
	"table.c")

# Tell cmake that we want 'libbmfs.so' and not 'libbmfs-library.so'
set_target_properties("bmfs-library" PROPERTIES OUTPUT_NAME "bmfs")

add_library("bmfs-stdlib-library"
	"filedisk.c"
	"ramdisk.c"
	"size.c"
	"time.c")

# Tell CMake that we want 'libbmfs-stdlib.so' and not 'libbmfs-stdlib-library.so'
set_target_properties("bmfs-stdlib-library" PROPERTIES OUTPUT_NAME "bmfs-stdlib")

install(TARGETS "bmfs-library" "bmfs-stdlib-library"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")
