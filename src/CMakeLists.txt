cmake_minimum_required(VERSION 2.6)

set(SRCDIR "${PROJECT_SOURCE_DIR}/src")
set(INCDIR "${PROJECT_SOURCE_DIR}/include/bmfs")

macro(ADD_BMFS_STRUCT VAR HEADER SOURCE)
	list(APPEND ${VAR}
		"${INCDIR}/${HEADER}"
		"${SRCDIR}/${SOURCE}")
	install(FILES "${INCDIR}/${HEADER}"
		DESTINATION "include/bmfs")
endmacro(ADD_BMFS_STRUCT VAR HEADER SOURCE)

unset(SOURCES)
add_bmfs_struct(SOURCES "entry.h" "entry.c")
add_bmfs_struct(SOURCES "explorer.h" "explorer.c")
add_bmfs_struct(SOURCES "dir.h" "dir.c")
add_bmfs_struct(SOURCES "disk.h" "disk.c")
add_bmfs_struct(SOURCES "path.h" "path.c")
add_bmfs_struct(SOURCES "sspec.h" "sspec.c")
add_library("bmfs-library" ${SOURCES})
# Tell cmake that we want 'libbmfs.so' and not 'libbmfs-library.so'
set_target_properties("bmfs-library" PROPERTIES OUTPUT_NAME "bmfs")
install(TARGETS "bmfs-library"
	ARCHIVE DESTINATION "lib"
	LIBRARY DESTINATION "lib")

add_library("bmfs-stdlib-library"
	"${INCDIR}/stdlib.h"
	"${SRCDIR}/stdlib.c")
# Tell CMake that we want 'libbmfs-stdlib' and not 'libbmfs-stdlib-library.so'
set_target_properties("bmfs-stdlib-library" PROPERTIES OUTPUT_NAME "bmfs-stdlib")
install(TARGETS "bmfs-stdlib-library"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")

macro(ADD_BMFS_PROGRAM PROGRAM_NAME)
	add_executable("${PROGRAM_NAME}" ${ARGN})
	target_link_libraries("${PROGRAM_NAME}"
		"bmfs-library"
		"bmfs-stdlib-library")
	install(TARGETS "${PROGRAM_NAME}"
		RUNTIME DESTINATION "bin")
endmacro(ADD_BMFS_PROGRAM PROGRAM_NAME)

add_bmfs_program("bmfs" "bmfs.c")

if (BMFS_WITH_POSIX_UTILS)
	add_bmfs_program("bmfs-cat" "bmfs-cat.c")
	add_bmfs_program("bmfs-cp" "bmfs-cp.c")
	add_bmfs_program("bmfs-create" "bmfs-create.c")
	add_bmfs_program("bmfs-init" "bmfs-init.c")
	add_bmfs_program("bmfs-ls" "bmfs-ls.c")
	add_bmfs_program("bmfs-rm" "bmfs-rm.c")
endif (BMFS_WITH_POSIX_UTILS)

if (BMFS_WITH_FUSE)
	add_bmfs_program("bmfs-fuse" "bmfs-fuse.c")
	target_link_libraries("bmfs-fuse" "fuse")
endif (BMFS_WITH_FUSE)

macro(ADD_BMFS_TEST TEST_NAME)
	add_bmfs_program("${TEST_NAME}" ${ARGN})
	add_test("${TEST_NAME}" "${TEST_NAME}")
endmacro(ADD_BMFS_TEST TEST_NAME)

add_bmfs_test("dir-test" "dir-test.c")
add_bmfs_test("disk-test" "disk-test.c")
add_bmfs_test("sspec-test" "sspec-test.c")
add_bmfs_test("path-test" "path-test.c")
enable_testing()
