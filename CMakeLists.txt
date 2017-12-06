cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)

project(HIDRA2)

option(BUILD_TESTS "Uses googletest to build tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests (CMAKE >3.7 is needed)" OFF)
option(BUILD_DOCS "Uses doxygen to build the documentaion" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

# format sources
include(astyle)

include(testing_cpp)

add_subdirectory(producer/api)
add_subdirectory(common/cpp)
add_subdirectory(producer/inotify-event-detector-cpp)
add_subdirectory(worker/api/cpp)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()



if(BUILD_DOCS)
    include(doxygen)
endif()
