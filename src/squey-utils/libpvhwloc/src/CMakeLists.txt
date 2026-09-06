
set(SRC_FILES
pvhwloc.cpp
)

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER ${HWLOC_INCLUDE_DIRS})

# dynamic library definition
add_library (pvhwloc SHARED ${SRC_FILES})
set_target_properties(pvhwloc PROPERTIES SOVERSION 1)
set_target_properties(pvhwloc PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}")

# the dependencies for the dynamic library
target_link_libraries(pvhwloc ${HWLOC_LIBRARIES})

# installation rules
install(TARGETS pvhwloc LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT lib)
