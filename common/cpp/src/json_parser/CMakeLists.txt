set(TARGET_NAME json_parser)
set(SOURCE_FILES
        json_string_parser.cpp
        rapid_json.cpp)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/3d_party/rapidjson/include)

################################
# Testing
################################


set(TEST_SOURCE_FILES ../../unittests/json_parser/test_json_string_parser.cpp)

set(TEST_LIBRARIES "${TARGET_NAME}")
include_directories(${HIDRA2_CXX_COMMON_INCLUDE_DIR})
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
