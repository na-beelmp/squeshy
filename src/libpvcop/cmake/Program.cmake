
# permits to easily declare a new target for a program given some dependencies:
# - its sources files
# - the target it depends on
# - extra librairies
#
# Warning:
# - it is restricted to C++ only
# - the supported librairies are: OpenMP (if found)
#
# The following variables are set:
#    PROGRAM_COMPILE_FLAGS - additional compilation flags
#    PROGRAM_LINK_FLAGS - additional link flags
#    PROGRAM_TARGET_DEPENDENCIES - additional deduced target dependencies
#    PROGRAM_TARGET_LIBRAIRIES - additional deduced librairies

macro(declare_program target)
	set(FILE_DEPS "")
	set(TARGET_DEPS "")
	set(TARGET_COMPILE_FLAGS "")
	set(TARGET_LINK_FLAGS "")
	set(TARGET_LIBS "")

	foreach(ARG IN ITEMS ${ARGN})
		if(OPENMP_FOUND AND ${ARG} STREQUAL "openmp")
			list(APPEND TARGET_COMPILE_FLAGS "${OpenMP_CXX_FLAGS}")
			list(APPEND TARGET_LINK_FLAGS "${OpenMP_CXX_FLAGS}")
		elseif(TBB_FOUND AND ${ARG} STREQUAL "tbbmalloc")
			list(APPEND TARGET_LINK_FLAGS "-ltbbmalloc")
		elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARG}")
			# a local file
			list(APPEND FILE_DEPS ${ARG})
		elseif(EXISTS "${PROJECT_SOURCE_DIR}/${ARG}")
			# a local file
			list(APPEND FILE_DEPS "${PROJECT_SOURCE_DIR}/${ARG}")
		else()
			# a target
			list(APPEND TARGET_DEPS ${ARG})
		endif()
	endforeach()

	add_executable(${target} EXCLUDE_FROM_ALL ${FILE_DEPS})
	set(PROGRAM_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS}" CACHE STRING "Program compile flags" FORCE)
	set(PROGRAM_LINK_FLAGS "${TARGET_LINK_FLAGS}" CACHE STRING "Program link flags" FORCE)
	set(PROGRAM_TARGET_DEPENDENCIES "${TARGET_DEPS}" CACHE STRING "Program target dependencies" FORCE)
	set(PROGRAM_TARGET_LIBRAIRIES "${TARGET_LIBS}" CACHE STRING "Program target librairies" FORCE)
endmacro()