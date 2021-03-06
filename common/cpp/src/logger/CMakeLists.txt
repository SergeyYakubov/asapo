set(TARGET_NAME logger)
set(SOURCE_FILES
        spd_logger.cpp spd_logger.h logger.cpp fluentd_sink.cpp fluentd_sink.h)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/3d_party/spd_log/include)

################################
# Testing
################################


set(TEST_SOURCE_FILES ../../unittests/logger/test_logger.cpp
        ../../unittests/logger/test_fluentd_sink.cpp)

link_libraries(CURL::libcurl ${CMAKE_THREAD_LIBS_INIT})

set(TEST_LIBRARIES "${TARGET_NAME};curl_http_client;system_io")
include_directories(${ASAPO_CXX_COMMON_INCLUDE_DIR})
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3d_party/spd_log/include)
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
