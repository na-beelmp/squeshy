cmake_minimum_required(VERSION 3.5)

project(furl-project)

include(CMakeOptions.txt)

set(CMAKE_BUILD_TYPE "Release")

if (DEBUG_MODE)
set(CMAKE_BUILD_TYPE "Debug")
	if(NOT WIN32)
		add_definitions(-g)
	endif()
else()
	if(NOT WIN32)
		if (NOT DEFINED ARCH_TYPE)
			if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
				set(ARCH_TYPE "x86-64-v3")
			endif()
		endif()
		if (DEFINED ARCH_TYPE)
			set(CMAKE_CXX_ARCH_FLAGS "-march=${ARCH_TYPE}")
		endif()

		add_definitions(-g -Os ${CMAKE_CXX_ARCH_FLAGS})
	endif()
endif()

add_definitions(-std=c99 -Wall -Wextra -fPIC)

include(CTest)

set(FURL_INCLUDE_DIRS "${furl-project_SOURCE_DIR}/src/lib/include/")


set(FURL_LIBRARY_UTF8 "${CMAKE_BINARY_DIR}/furl/src/lib/libfurls-utf8${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(FURL_LIBRARY_UTF16 "${CMAKE_BINARY_DIR}/furl/src/lib/libfurls-utf16${CMAKE_SHARED_LIBRARY_SUFFIX}")


add_subdirectory(src)
