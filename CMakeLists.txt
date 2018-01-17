cmake_minimum_required(VERSION 2.8)
project(HIDRA2)
set(CMAKE_CXX_STANDARD 11)
IF(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
ENDIF(WIN32)

option(BUILD_TESTS "Uses googletest to build tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests (CMAKE >3.7 is needed)" OFF)
option(BUILD_DOCS "Uses doxygen to build the documentaion" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

# format sources
include(astyle)

include(testing_cpp)

add_subdirectory(producer/api)
add_subdirectory(common/cpp)
add_subdirectory(worker/api/cpp)
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
