cmake_minimum_required(VERSION 2.6)

project("BMFS" C)

set (BMFS_VERSION_MAJOR 1)
set (BMFS_VERSION_MINOR 3)
set (BMFS_VERSION_PATCH 0)

option(BMFS_WITH_FUSE "Build BMFS with fuse support" OFF)
option(BMFS_WITH_POSIX_UTILS "Build BMFS POSIX utilities" OFF)

set (GNU_C_FLAGS "-Wall -Wextra -Werror -Wfatal-errors -std=gnu99")
set (GNU_C_FLAGS "${GNU_C_FLAGS} -D_FILE_OFFSET_BITS=64")

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set (CMAKE_C_FLAGS ${GNU_C_FLAGS})
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	set (CMAKE_C_FLAGS ${GNU_C_FLAGS})
endif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

if (MSVC)
	set (CMAKE_C_FLAGS "/W3 /WX /D_CRT_SECURE_NO_WARNINGS")
endif (MSVC)

include_directories("include")

add_subdirectory("lib")
add_subdirectory("utils")
add_subdirectory("tests")

enable_testing()

set(CPACK_PACKAGE_NAME "BMFS")
set(CPACK_DESCRIPTION_SUMMARY "A collection of utilities of the BMFS file system.")
set(CPACK_PACKAGE_VENDOR "ReturnInfinity")
set(CPACK_PACKAGE_VERSION_MAJOR ${BMFS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BMFS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BMFS_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_CONTACT "taylorcholberton@gmail.com")
set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)
