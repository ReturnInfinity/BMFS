cmake_minimum_required(VERSION 2.6)

macro(ADD_BMFS_TEST TEST)
	add_executable(${TEST} ${ARGN})
	target_link_libraries(${TEST} "bmfs-library" "bmfs-stdlib-library")
	add_test(${TEST} ${TEST})
endmacro(ADD_BMFS_TEST TEST)

add_bmfs_test("dir-test" "dir-test.c")

add_bmfs_test("disk-test" "disk-test.c")

add_bmfs_test("encoding-test" "encoding-test.c")

add_bmfs_test("fs-test" "fs-test.c")

add_bmfs_test("path-test" "path-test.c")

add_bmfs_test("sizeof-test" "sizeof-test.c")

add_bmfs_test("size-test" "size-test.c")

add_bmfs_test("table-test" "table-test.c")

enable_testing()
