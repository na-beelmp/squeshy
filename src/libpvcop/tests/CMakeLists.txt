# Default values for tests files root
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_definitions(-DTESTS_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/files")
else()
add_definitions(-DTESTS_FILES_DIR="../../../../../../../src/libpvcop/tests/files")
endif()

add_custom_target(pvcop_bench COMMAND ${CMAKE_CTEST_COMMAND} -R PVCOP_BENCH --schedule-random)

add_custom_target(pvcop_testsuite)
if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(pvcop_run_testsuite COMMAND ${CMAKE_CTEST_COMMAND} -R PVCOP_TEST_ 
        --force-new-ctest-process --output-on-failure
        --build-config "$<CONFIGURATION>")
else()
    add_custom_target(pvcop_run_testsuite COMMAND ${CMAKE_CTEST_COMMAND} -R PVCOP_TEST_ 
        --force-new-ctest-process --output-on-failure)
endif()
add_dependencies(pvcop_run_testsuite pvcop_testsuite)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/update-tests-files.sh")
add_custom_target(update_tests_files COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/update_tests_files.sh")
add_dependencies(pvcop_testsuite update_tests_files)

set(TESTSUITE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(OpenMP_LIBRARIES "-fopenmp=libiomp5")
  add_definitions("-fopenmp=libiomp5")
else()
  find_package(OpenMP REQUIRED)
  set(OpenMP_LIBRARIES ${OpenMP_CXX_FLAGS})
  add_definitions(${OpenMP_CXX_FLAGS})
endif()

macro(declare_bench target)
    add_dependencies(pvcop_bench PVCOP_BENCH_${target})

	declare_program(PVCOP_BENCH_${target} ${ARGN})

	target_link_libraries(PVCOP_BENCH_${target} pvcop  ${OpenMP_LIBRARIES} ${ICU_LIBRARIES})
	set_target_properties(PVCOP_BENCH_${target} PROPERTIES COMPILE_FLAGS "-I${TESTSUITE_ROOT_DIR} -I${PROJECT_SOURCE_DIR}/src -DTEST_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${target}\" -DPVCOP_BENCH ${PROGRAM_COMPILE_FLAGS}")
	set_target_properties(PVCOP_BENCH_${target} PROPERTIES LINK_FLAGS "${PROGRAM_LINK_FLAGS}")

	add_test(PVCOP_BENCH_${target} ${PROJECT_SOURCE_DIR}/scripts/run_bench.py ${CMAKE_CURRENT_BINARY_DIR}/PVCOP_BENCH_${target})
	set_property(TEST PVCOP_BENCH_${target} PROPERTY ENVIRONMENT "PVLOGGER_LEVEL=FATAL")
endmacro()

macro(declare_test target)
    add_dependencies(pvcop_testsuite PVCOP_TEST_${target})

    declare_program(PVCOP_TEST_${target} ${ARGN})

    target_link_libraries(PVCOP_TEST_${target} pvcop ${OpenMP_LIBRARIES} ${ICU_LIBRARIES})
    set_target_properties(PVCOP_TEST_${target} PROPERTIES COMPILE_FLAGS "-I${TESTSUITE_ROOT_DIR} -I${PROJECT_SOURCE_DIR}/src -DTEST_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${target}\" ${PROGRAM_COMPILE_FLAGS}")
    set_target_properties(PVCOP_TEST_${target} PROPERTIES LINK_FLAGS "${PROGRAM_LINK_FLAGS}")

    add_test(PVCOP_TEST_${target} PVCOP_TEST_${target})
    set_property(TEST PVCOP_TEST_${target} PROPERTY ENVIRONMENT "PVLOGGER_LEVEL=FATAL")
endmacro()

add_subdirectory(core)
add_subdirectory(db)
add_subdirectory(types)
add_subdirectory(pvcop)
