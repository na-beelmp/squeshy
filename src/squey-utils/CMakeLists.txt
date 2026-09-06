cmake_minimum_required(VERSION 3.5)

include_directories("/mac/include")

project(squey-utils)
set(SQUEY_UTILS true PARENT_SCOPE) # Useful to detect if this project is already imported

include(CTest)

include(CMakeOptions.txt)
include(CMakeCompilers.txt)

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
message(STATUS "Architecture flag: ${CMAKE_CXX_ARCH_FLAGS}")

if(WITH_PVLOGGER)
  add_subdirectory(libpvlogger)
endif()

if(WITH_PVHWLOC)
  add_subdirectory(libpvhwloc)
endif()

if(WITH_SQUEY_UTILS_CORE)
  add_subdirectory(core)
endif()

###############################################################################
# Add test targets
###############################################################################

add_custom_target(squey-utils_testsuite)
add_dependencies(squey-utils_testsuite testsuite_logger)
add_dependencies(squey-utils_testsuite testsuite_hwloc)
