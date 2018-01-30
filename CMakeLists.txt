cmake_minimum_required(VERSION 2.8)
project(HIDRA2)
set(CMAKE_CXX_STANDARD 11)
IF(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(WIN32)

option(BUILD_TESTS "Uses googletest to build tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests (CMAKE >3.7 is needed)" OFF)
option(BUILD_DOCS "Uses doxygen to build the documentaion" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

set (HIDRA2_CXX_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/cpp/include)

# format sources
include(astyle)

include(testing_cpp)

if(BUILD_WORKER_TOOLS)
    set (BUILD_MONGODB_CLIENTLIB ON)
endif()

add_subdirectory(common/cpp)

add_subdirectory(producer/api)

add_subdirectory(worker)


IF(UNIX)
    add_subdirectory(receiver)
ENDIF(UNIX)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_DOCS)
    include(doxygen)
endif()


if (BUILD_BROKER)
    add_subdirectory(broker)
endif()