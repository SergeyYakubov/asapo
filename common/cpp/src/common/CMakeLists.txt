set(TARGET_NAME common)

set(TEST_SOURCE_FILES ../../unittests/common/test_error.cpp)

set(TEST_LIBRARIES "${TARGET_NAME};system_io")
include_directories(${HIDRA2_CXX_COMMON_INCLUDE_DIR})
add_plain_unit_test(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
