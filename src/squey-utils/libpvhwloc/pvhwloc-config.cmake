# - Try to find PVHWLoc
# Once done this will define
#  PVHWLOC_FOUND - System has PVHWLoc
#  PVHWLOC_INCLUDE_DIRS - The PVHWLoc include directories
#  PVHWLOC_LIBRARIES - The libraries needed to use PVHWLoc
#  PVHWLOC_DEFINITIONS - Compiler switches required for using PVHWLoc

find_path(PVHWLOC_INCLUDE_DIR pvhwloc.h
          HINTS /usr/include /app/include /win/include /mac/include
          PATH_SUFFIXES pvhwloc)

find_library(PVHWLOC_LIBRARY NAMES pvhwloc libpvhwloc
             HINTS /mac/lib)

set(PVHWLOC_LIBRARIES ${PVHWLOC_LIBRARY} )
set(PVHWLOC_INCLUDE_DIRS ${PVHWLOC_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PVHWLOC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(PVHWloc  DEFAULT_MSG
                                  PVHWLOC_LIBRARY PVHWLOC_INCLUDE_DIR)

mark_as_advanced(PVHWLOC_INCLUDE_DIR PVHWLOC_LIBRARY)
