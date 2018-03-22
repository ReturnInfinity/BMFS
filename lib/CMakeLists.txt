cmake_minimum_required(VERSION 2.6)

add_library("bmfs-library"
	"entry.c"
	"dir.c"
	"disk.c"
	"header.c"
	"path.c"
	"ramdisk.c"
	"table.c")

# Tell cmake that we want 'libbmfs.so' and not 'libbmfs-library.so'
set_target_properties("bmfs-library" PROPERTIES OUTPUT_NAME "bmfs")

add_library("bmfs-stdlib-library"
	"sspec.c"
	"stdlib.c"
	"time.c")

# Tell CMake that we want 'libbmfs-stdlib.so' and not 'libbmfs-stdlib-library.so'
set_target_properties("bmfs-stdlib-library" PROPERTIES OUTPUT_NAME "bmfs-stdlib")

install(TARGETS "bmfs-library" "bmfs-stdlib-library"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")
