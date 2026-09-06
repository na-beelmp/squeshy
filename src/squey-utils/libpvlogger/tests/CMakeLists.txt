add_custom_target(testsuite_logger)

set(TESTSUITE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

macro(declare_test target)
	add_dependencies(testsuite_logger ${target})
	add_executable(${target} EXCLUDE_FROM_ALL ${ARGN})
	target_link_libraries(${target} pvlogger)
	set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-I${TESTSUITE_ROOT_DIR}/common -DTEST_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${target}\"")
	
	add_test(${target} ${target})
	set_property(TEST ${target} PROPERTY ENVIRONMENT "PVLOGGER_LEVEL=FATAL")
endmacro()

declare_test(hello_world hello_world/main.cpp)
