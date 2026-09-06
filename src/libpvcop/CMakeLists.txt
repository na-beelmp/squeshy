cmake_minimum_required(VERSION 3.5)

project(libpvcop)

include(CTest)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

###############################################################################
# Define the Intel TBB Library
###############################################################################
include(CMakeOptions.txt)
include(CMakeCompilers.txt)
include(cmake/Program.cmake)

find_package(Doxygen)
find_package(OpenMP)
find_package(TBB "2021.0" REQUIRED)
set(TBB_LIBRARIES "TBB::tbb;TBB::tbbmalloc")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  SET(Tcmalloc_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
  find_package(Tcmalloc REQUIRED)
endif()
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
SET(BOOST_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
find_package(Boost 1.53 COMPONENTS date_time)


###############################################################################
# Python3 + Numpy
###############################################################################
find_package(Python3 REQUIRED COMPONENTS Development)
include_directories(AFTER SYSTEM ${Python3_INCLUDE_DIRS})
find_package (Python3 COMPONENTS Interpreter NumPy)
include_directories(AFTER SYSTEM ${Python_NumPy_INCLUDE_DIRS})

###############################################################################
# squey-utils
###############################################################################
if (NOT DEFINED SQUEY_UTILS)
	message(STATUS "Configuring squey-utils...")
	SET(PVLOGGER_LIBRARIES pvlogger)
	SET(PVHWLOC_LIBRARIES pvhwloc)
	include_directories(${CMAKE_SOURCE_DIR}/squey-utils/include)
	include_directories(${CMAKE_SOURCE_DIR}/squey-utils/libpvlogger/include)
	include_directories(${CMAKE_SOURCE_DIR}/squey-utils/libpvhwloc/include)
	add_subdirectory(squey-utils)
endif()

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
message(STATUS "Architecture flag: ${CMAKE_CXX_ARCH_FLAGS}")
message(STATUS "use 64 bits indexes: ${INDEXES_64BITS}")

if(DOXYGEN_FOUND)
	message(STATUS "Doxygen found")
	add_subdirectory(doc)
endif(DOXYGEN_FOUND)

find_package(ICU 57.1 COMPONENTS data i18n uc REQUIRED)
if(ICU_FOUND)
	message(STATUS "ICU include: ${ICU_INCLUDE}")
  message(STATUS "ICU libraries: ${ICU_LIBRARY}")
endif(ICU_FOUND)

if (APPLE AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(OpenMP_LIBRARIES "/opt/homebrew/opt/libomp/lib/libomp.dylib")
  add_compile_options(-Xclang -fopenmp)
  include_directories(SYSTEM /opt/homebrew/opt/libomp/include)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(OpenMP_LIBRARIES "-fopenmp=libiomp5")
  add_definitions("-fopenmp=libiomp5")
else()
  find_package(OpenMP REQUIRED)
  set(OpenMP_LIBRARIES ${OpenMP_CXX_FLAGS})
  add_definitions(${OpenMP_CXX_FLAGS})
endif()

include_directories(AFTER include)
include_directories(AFTER third_party)
include_directories(AFTER SYSTEM ${TBB_INCLUDE_DIRS})
include_directories(AFTER ${ICU_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(include)

add_subdirectory(tests)

INSTALL(FILES pvcop-config.cmake DESTINATION lib/pvcop/cmake/ COMPONENT dev)
