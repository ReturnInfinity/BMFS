cmake_minimum_required(VERSION 2.6)

function (ADD_BMFS_UTIL UTIL)

	add_executable(${UTIL} ${ARGN})

	target_link_libraries(${UTIL} "bmfs-library" "bmfs-stdlib-library")

	set_target_properties(${UTIL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

endfunction (ADD_BMFS_UTIL UTIL)

add_bmfs_util("bmfs" "bmfs.c")

install(TARGETS "bmfs"
	EXPORT "bmfs-exports"
	RUNTIME DESTINATION "bin")

if (BMFS_WITH_FUSE)
	add_bmfs_util("bmfs-fuse" "bmfs-fuse.c")
	target_link_libraries("bmfs-fuse" "fuse")
	install(TARGETS "bmfs-fuse"
		EXPORT "bmfs-exports"
		RUNTIME DESTINATION "bin")
endif (BMFS_WITH_FUSE)
