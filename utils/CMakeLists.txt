cmake_minimum_required(VERSION 2.6)

function (ADD_BMFS_UTIL UTIL)
	add_executable(${UTIL} ${ARGN})
	target_link_libraries(${UTIL} "bmfs-library" "bmfs-stdlib-library")
endfunction (ADD_BMFS_UTIL UTIL)

add_bmfs_util("bmfs" "bmfs.c")

if (BMFS_WITH_POSIX_UTILS)
	add_bmfs_util("bmfs-cat" "bmfs-cat.c")
	add_bmfs_util("bmfs-cp" "bmfs-cp.c")
	add_bmfs_util("bmfs-create" "bmfs-create.c")
	add_bmfs_util("bmfs-init" "bmfs-init.c")
	add_bmfs_util("bmfs-ls" "bmfs-ls.c")
	add_bmfs_util("bmfs-rm" "bmfs-rm.c")
endif (BMFS_WITH_POSIX_UTILS)

if (BMFS_WITH_FUSE)
	add_bmfs_util("bmfs-fuse" "bmfs-fuse.c")
	target_link_libraries("bmfs-fuse" "fuse")
endif (BMFS_WITH_FUSE)
