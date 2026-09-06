
include_directories(AFTER ${FURL_INCLUDE_DIRS})

add_executable(furl-utf8 furl.c)
add_dependencies(furl-utf8 furll-utf8)
target_link_libraries(furl-utf8 ${FURL_LIBRARY_UTF8})

add_executable(furl-utf16 furl.c)
add_dependencies(furl-utf16 furll-utf16)
target_link_libraries(furl-utf16 ${FURL_LIBRARY_UTF16})
set_target_properties(furl-utf16 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DFURL_UTF16_CHAR")


