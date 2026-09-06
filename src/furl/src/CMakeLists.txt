
set(FURL_TOOLS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tools)
set(FURL_TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
set(FURL_TESTS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(tests)