if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/api.doxygen.in ${CMAKE_CURRENT_BINARY_DIR}/api.doxygen @ONLY)
	add_custom_target(api ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/api.doxygen
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	                  COMMENT "Generating API documentation" VERBATIM)

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/internal.doxygen.in ${CMAKE_CURRENT_BINARY_DIR}/internal.doxygen @ONLY)
	add_custom_target(internal ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/internal.doxygen
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	                  COMMENT "Generating internal documentation" VERBATIM)

	add_custom_target(doc DEPENDS api internal)
endif(DOXYGEN_FOUND)
