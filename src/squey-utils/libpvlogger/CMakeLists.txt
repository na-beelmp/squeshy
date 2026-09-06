project(libpvlogger)

include_directories(AFTER include)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(tests)

#INSTALL(FILES pvlogger-config.cmake DESTINATION lib/pvlogger/cmake/ COMPONENT dev)
