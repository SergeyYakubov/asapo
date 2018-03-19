cmake_minimum_required(VERSION 2.8)
project(HIDRA2)
set(CMAKE_CXX_STANDARD 11)
IF(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(WIN32)

#TODO: Better way then GLOBAL PROPERTY
IF(WIN32)
    find_package(Threads REQUIRED)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32)
ELSEIF(UNIX)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES Threads::Threads)
ENDIF(WIN32)

option(BUILD_TESTS "Uses googletest to build tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests (CMAKE >3.7 is needed)" OFF)
option(BUILD_DOCS "Uses doxygen to build the documentaion" OFF)
option(BUILD_BROKER "Build broker" OFF)
option(BUILD_WORKER_TOOLS "Build worker tools" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

set (HIDRA2_CXX_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/cpp/include)

find_package (Threads)


# format sources
include(astyle)

include(testing_cpp)

if(BUILD_WORKER_TOOLS)
    set (BUILD_MONGODB_CLIENTLIB ON)
endif()

add_subdirectory(common/cpp)

if (BUILD_BROKER)#TODO: Somehow make it clear that this is needed by examples/worker/getnext_broker
    add_subdirectory(broker)
endif()

add_subdirectory(producer/api)

add_subdirectory(worker)

add_subdirectory(receiver)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_DOCS)
    include(doxygen)
endif()

