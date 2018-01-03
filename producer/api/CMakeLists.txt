set(TARGET_NAME producer-api)
set(SOURCE_FILES src/producer/producer.cpp include/producer/producer.h)


################################
# Library
################################
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} $<TARGET_OBJECTS:common>)
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_producer.cpp)
set(TEST_LIBRARIES producer-api)


gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} ${TEST_LIBRARIES})


