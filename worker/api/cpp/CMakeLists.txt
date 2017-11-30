set(TARGET_NAME worker-api)




################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_worker_api.cpp)
set(TEST_LIBRARIES producer-api)


gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} ${TEST_LIBRARIES})