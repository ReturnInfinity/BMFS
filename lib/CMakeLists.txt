cmake_minimum_required(VERSION 2.6)

if (CMAKE_COMPILER_IS_GNUCC)
	# Set these flags incase the library is linked
	# with kernel code.
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-red-zone")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector -fomit-frame-pointer")
endif (CMAKE_COMPILER_IS_GNUCC)

add_library("bmfs-library"
	"crc32.c"
	"encoding.c"
	"entry.c"
	"errno.c"
	"dir.c"
	"disk.c"
	"file.c"
	"fs.c"
	"header.c"
	"host.c"
	"memcpy.c"
	"path.c"
	"status.c"
	"table.c")

# Tell cmake that we want 'libbmfs.so' and not 'libbmfs-library.so'
set_target_properties("bmfs-library" PROPERTIES OUTPUT_NAME "bmfs")

add_library("bmfs-stdlib-library"
	"filedisk.c"
	"ramdisk.c"
	"size.c"
	"stdhost.c"
	"time.c")

# Tell CMake that we want 'libbmfs-stdlib.so' and not 'libbmfs-stdlib-library.so'
set_target_properties("bmfs-stdlib-library" PROPERTIES OUTPUT_NAME "bmfs-stdlib")

install(TARGETS "bmfs-library" "bmfs-stdlib-library"
		EXPORT "bmfs-exports"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")
