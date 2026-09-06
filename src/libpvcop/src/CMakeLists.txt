
# code namespace's files
set(CORE_SRC_FILES
core/impl/type_traits.cpp
core/algo/selection.cpp
core/algo/bitwise_operators.cpp
)

# db namespace's files
SET(DB_SRC_FILES
db/array/array_base.cpp
db/array/array.cpp

db/array/exceptions/partially_converted_error.cpp

db/algo/group_by_operations.cpp
db/algo/math_operations.cpp
db/algo/subselect.cpp
db/algo/range_select.cpp

db/types/types_manager.cpp

db/collection/collection.cpp
db/collection/collector.cpp
db/collection/sink.cpp
db/collection/format.cpp
db/collection/file_handler.cpp
db/collection/file_read_handler.cpp
db/collection/file_write_handler.cpp
db/collection/description.cpp
db/collection/read_dict.cpp
db/collection/write_dict.cpp
db/collection/sel_write_handler.cpp
)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set (DB_SRC_FILES ${DB_SRC_FILES}
    db/collection/posix_fallocate.cpp
  )
endif()

SET(TYPES_SRC_FILES
types/datetime.cpp
types/datetime_ms.cpp
types/datetime_us.cpp
types/duration.cpp
types/ipv4.cpp
types/ipv6.cpp
types/mac_address.cpp
types/impl/number_adapter.cpp
types/impl/formatter_factory.cpp
types/impl/factory.cpp
${STRING_TO_NUMBER_IMPL}
collector.cpp
sink.cpp
formatter_desc_list.cpp
collection.cpp
sel_read_handler.cpp
)

# utils files
SET(UTILS_SRC_FILES
utils/filesystem.cpp
)

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER SYSTEM ${TBB_INCLUDE_DIRS})
include_directories(AFTER ${PVHWLOC_INCLUDE_DIRS})
include_directories(AFTER ${PVLOGGER_INCLUDE_DIRS})

# dynamic library definition
add_library (pvcop SHARED ${CORE_SRC_FILES} ${DB_SRC_FILES} ${TYPES_SRC_FILES} ${UTILS_SRC_FILES})
set_target_properties(pvcop PROPERTIES SOVERSION 1)

# the dependencies for the dynamic library
set(PVCOP_LIBRARIES ${TBB_LIBRARIES} ${PVHWLOC_LIBRARIES} ${PVLOGGER_LIBRARIES} ${ICU_LIBRARIES} ${OpenMP_LIBRARIES} ${Python3_LIBRARIES})
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  SET(PVCOP_LIBRARIES ${PVCOP_LIBRARIES} "-lws2_32")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  SET(PVCOP_LIBRARIES ${PVCOP_LIBRARIES} ${Tcmalloc_LIBRARY})
endif()

target_link_libraries(pvcop ${PVCOP_LIBRARIES})

# installation rules
install(TARGETS pvcop LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT lib)
