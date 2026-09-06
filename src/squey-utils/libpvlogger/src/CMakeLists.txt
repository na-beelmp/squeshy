
set(SRC_FILES
pvlogger.cpp
)

# make OBJECT cmake target to compile .o files only one time
##add_library(libobject OBJECT ${SRC_FILES})

# dynamic library definition
add_library (pvlogger SHARED ${SRC_FILES})
set_target_properties(pvlogger PROPERTIES SOVERSION 1)
set_target_properties(pvlogger PROPERTIES COMPILE_FLAGS "-DNDEBUG  ${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}")


# the dependencies for the dynamic library
target_link_libraries(pvlogger)

# installation rules
install(TARGETS pvlogger LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT lib)
