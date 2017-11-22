cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)

project(HIDRA2)
project(${PROJECT_NAME} C CXX)

enable_testing()

set(TEST_SOURCE_FILES unittests/test-suite.cpp)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_executable(test-${PROJECT_NAME} ${TEST_SOURCE_FILES})

target_link_libraries(test-${PROJECT_NAME} gtest gtest_main pthread)

#target_link_libraries(test-${PROJECT_NAME} cpp-producer)

add_test(NAME unittests COMMAND test-${PROJECT_NAME})

#astyle(test-${PROJECT_NAME} ${TEST_SOURCE_FILES})