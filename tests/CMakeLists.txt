cmake_minimum_required(VERSION 2.6)

macro(ADD_BMFS_TEST TEST)
	add_executable(${TEST} ${ARGN})
	target_link_libraries(${TEST} "bmfs-library" "bmfs-stdlib-library")
	add_test(${TEST} ${TEST})
endmacro(ADD_BMFS_TEST TEST)

add_bmfs_test("dir-test" "dir.c")

add_bmfs_test("disk-test" "disk.c")

add_bmfs_test("fs-test" "fs.c")

add_bmfs_test("path-test" "path.c")

add_bmfs_test("size-test" "size.c")

add_bmfs_test("sspec-test" "sspec.c")

add_bmfs_test("table-test" "table.c")

enable_testing()
