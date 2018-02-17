set(TARGET_NAME data_structs)
set(SOURCE_FILES
        data_structs.cpp
)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/3d_party/rapidjson/include)

################################
# Testing
################################
set(TEST_SOURCE_FILES ../../unittests/data_structs/test_data_structs.cpp)

set(TEST_LIBRARIES "${TARGET_NAME};json_parser;system_io")
include_directories(${HIDRA2_CXX_COMMON_INCLUDE_DIR})
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")

