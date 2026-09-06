add_custom_target(testsuite_hwloc)

set(TESTSUITE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

macro(declare_test target)
	add_dependencies(testsuite_hwloc ${target})
	add_executable(${target} EXCLUDE_FROM_ALL ${ARGN})
	target_link_libraries(${target} pvhwloc)
	set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-I${TESTSUITE_ROOT_DIR}/common -DTEST_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${target}\"")
	
	add_test(${target} ${target})
endmacro()

declare_test(main main.cpp)
