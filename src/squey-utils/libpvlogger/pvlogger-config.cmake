# - Try to find PVLogger
# Once done this will define
#  PVLOGGER_FOUND - System has PVLogger
#  PVLOGGER_INCLUDE_DIRS - The PVLogger include directories
#  PVLOGGER_LIBRARIES - The libraries needed to use PVLogger
#  PVLOGGER_DEFINITIONS - Compiler switches required for using PVLogger

find_package(PkgConfig)
find_path(PVLOGGER_INCLUDE_DIR pvlogger.h
          HINTS /usr/include
          PATH_SUFFIXES pvlogger)

find_library(PVLOGGER_LIBRARY NAMES pvlogger libpvlogger
             HINTS /usr/lib )

set(PVLOGGER_LIBRARIES ${PVLOGGER_LIBRARY} )
set(PVLOGGER_INCLUDE_DIRS ${PVLOGGER_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PVLOGGER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(PVLogger  DEFAULT_MSG
                                  PVLOGGER_LIBRARY PVLOGGER_INCLUDE_DIR)

mark_as_advanced(PVLOGGER_INCLUDE_DIR PVLOGGER_LIBRARY)
