include_directories(AFTER "include/")

set(FURL_LIB_SRC_FILES
decode.c
features.c
furl.c
)

set(FURL_LIB_HDR_FILES
include/furl/features.h
include/furl/furl.h
include/furl/decode.h
)

set(FURL_LINK_LIBRARIES
)

set(FURL_SOVERSION 1)

# Our static library
add_library(furll-utf8 ${FURL_LIB_SRC_FILES})
set_target_properties(furll-utf8 PROPERTIES SOVERSION ${FURL_SOVERSION})
target_link_libraries(furll-utf8 ${FURL_LINK_LIBRARIES})

add_library(furll-utf16 ${FURL_LIB_SRC_FILES})
set_target_properties(furll-utf16 PROPERTIES SOVERSION ${FURL_SOVERSION})
target_link_libraries(furll-utf16 ${FURL_LINK_LIBRARIES})
set_target_properties(furll-utf16 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DFURL_UTF16_CHAR")

# Our dynamic library
add_library(furls-utf8 SHARED ${FURL_LIB_SRC_FILES})
set_target_properties(furls-utf8 PROPERTIES SOVERSION ${FURL_SOVERSION})
target_link_libraries(furls-utf8 ${FURL_LINK_LIBRARIES})

add_library(furls-utf16 SHARED ${FURL_LIB_SRC_FILES})
set_target_properties(furls-utf16 PROPERTIES SOVERSION ${FURL_SOVERSION})
target_link_libraries(furls-utf16 ${FURL_LINK_LIBRARIES})
set_target_properties(furls-utf16 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DFURL_UTF16_CHAR")

set(LEGACY_SHARED libfurls${CMAKE_SHARED_LIBRARY_SUFFIX})
add_custom_command(TARGET furls-utf16 POST_BUILD
    COMMAND ln -sf libfurls-utf16${CMAKE_SHARED_LIBRARY_SUFFIX} ${LEGACY_SHARED}
)
if (WIN32)
set(DEST_DIR "bin")
else()
set(DEST_DIR "lib")
endif()

set(LEGACY_STATIC libfurll.a)

add_custom_command(TARGET furll-utf16 POST_BUILD
    COMMAND ln -sf libfurll-utf16.a ${LEGACY_STATIC}
)

# install
INSTALL(TARGETS furll-utf8 ARCHIVE DESTINATION ${DEST_DIR})
INSTALL(TARGETS furls-utf8 LIBRARY DESTINATION ${DEST_DIR})
INSTALL(TARGETS furll-utf16 ARCHIVE DESTINATION ${DEST_DIR})
INSTALL(TARGETS furls-utf16 LIBRARY DESTINATION ${DEST_DIR})
INSTALL(DIRECTORY include/furl DESTINATION include)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LEGACY_STATIC} DESTINATION ${DEST_DIR})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LEGACY_SHARED} DESTINATION ${DEST_DIR})



