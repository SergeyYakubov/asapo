set(TARGET_NAME common)

set(SOURCE_FILES common_c_glue.cpp)

set(TEST_SOURCE_FILES ../../unittests/common/test_error.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})

set(TEST_LIBRARIES "${TARGET_NAME};system_io")
include_directories(${ASAPO_CXX_COMMON_INCLUDE_DIR} ../../include)
add_plain_unit_test(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
