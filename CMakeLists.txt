cmake_minimum_required(VERSION 2.6)

project("BMFS")

set (BMFS_VERSION_MAJOR 1)
set (BMFS_VERSION_MINOR 2)
set (BMFS_VERSION_PATCH 3)

option(BMFS_WITH_FUSE "Build BMFS with fuse support" OFF)
option(BMFS_WITH_POSIX_UTILS "Build BMFS POSIX utilities" OFF)

if (CMAKE_COMPILER_IS_GNUCC)
	set (CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wfatal-errors")
endif (CMAKE_COMPILER_IS_GNUCC)

if (MSVC)
	set (CMAKE_C_FLAGS "/W3 /WX /D_CRT_SECURE_NO_WARNINGS")
endif (MSVC)

include_directories("include")

add_subdirectory("src")

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
