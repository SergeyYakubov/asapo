set(TARGET_NAME logger)
set(SOURCE_FILES
        spd_logger.cpp spd_logger.h logger_factory.cpp fluentd_sink.cpp fluentd_sink.h)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/3d_party/spd_log/include)

################################
# Testing
################################


set(TEST_SOURCE_FILES ../../unittests/logger/test_logger.cpp)

set(TEST_LIBRARIES "${TARGET_NAME};curl_http_client;${CURL_LIBRARIES}")
include_directories(${HIDRA2_CXX_COMMON_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/3d_party/spd_log/include)
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
