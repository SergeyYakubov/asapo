cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)

project(HIDRA2)

option(BUILD_TESTS "Uses googletest to build tests" OFF)
option(BUILD_DOCS "Uses doxygen to build the documentaion" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

include(astyle)

if(BUILD_TESTS)
	include(testing_cpp)
endif()

add_subdirectory(producer/api)
add_subdirectory(common/cpp)
add_subdirectory(producer/inotify-event-detector-cpp)


if(BUILD_DOCS)
    include(doxygen)
endif()
