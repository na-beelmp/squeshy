project(libpvhwloc)

find_package(PkgConfig)
pkg_check_modules(HWLOC REQUIRED hwloc)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(tests)

#INSTALL(FILES pvhwloc-config.cmake DESTINATION lib/pvhwloc/cmake/ COMPONENT dev)
