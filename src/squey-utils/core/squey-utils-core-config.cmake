# - Try to find SQUEY_UTILS_CORE

find_package(PkgConfig)

set(SQUEY_UTILS_CORE_LIBRARIES ${SQUEY_UTILS_CORE_LIBRARY} dl )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PVLOGGER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SQUEY_UTILS_CORE DEFAULT_MSG
          SQUEY_UTILS_CORE_LIBRARIES SQUEY_UTILS_CORE_INCLUDE_DIR)

message(STATUS "SQUEY_UTILS_CORE include dirs: ${SQUEY_UTILS_CORE_INCLUDE_DIR}")
message(STATUS "SQUEY_UTILS_CORE libraries: ${SQUEY_UTILS_CORE_LIBRARIES}")

mark_as_advanced(SQUEY_UTILS_CORE_INCLUDE_DIR SQUEY_UTILS_CORE_LIBRARIES)
