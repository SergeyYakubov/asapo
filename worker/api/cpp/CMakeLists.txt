set(TARGET_NAME worker-api)

set(SOURCE_FILES include/worker/data_source.h src/data_broker_factory.cpp src/folder_data_broker.cpp src/folder_data_broker.h)


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} common)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_worker_api.cpp)
set(TEST_LIBRARIES ${TARGET_NAME})


gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} "${TEST_LIBRARIES}")