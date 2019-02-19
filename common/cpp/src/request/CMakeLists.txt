set(TARGET_NAME request_pool)
set(SOURCE_FILES
        request_pool.cpp)

################################
# Library
################################

add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})

target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################


set(TEST_SOURCE_FILES ../../unittests/request/test_request_pool.cpp
        ../../unittests/request/test_request.cpp)

link_libraries(${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

set(TEST_LIBRARIES "${TARGET_NAME};system_io;logger;curl_http_client")
include_directories(${ASAPO_CXX_COMMON_INCLUDE_DIR})
gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}")
