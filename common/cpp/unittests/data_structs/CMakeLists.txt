set(TARGET_NAME data_structs)


################################
# Testing
################################
set(TEST_SOURCE_FILES test_data_structs.cpp)

set(TEST_LIBRARIES "${TARGET_NAME}")
include_directories(${HIDRA2_CXX_COMMON_INCLUDE_DIR})
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

