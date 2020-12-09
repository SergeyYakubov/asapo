cmake_minimum_required(VERSION 3.7)
project(ASAPO)
set(CMAKE_CXX_STANDARD 11)
IF(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    add_definitions(-DWIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
#todo: added fPIC to be able to create shared libs for Python - check it does not change performance
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -fPIC")
    SET(BUILD_SHARED_LIBS OFF)
ENDIF(WIN32)

#TODO: Better way then GLOBAL PROPERTY
IF(WIN32)
    find_package(Threads REQUIRED)
    SET(ASAPO_COMMON_IO_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32)
ELSEIF(UNIX)
    SET(ASAPO_COMMON_IO_LIBRARIES Threads::Threads)
ENDIF(WIN32)
SET_PROPERTY(GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES ${ASAPO_COMMON_IO_LIBRARIES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DUNIT_TESTS)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

if (APPLE)
    link_directories("/usr/local/lib")
endif()

option(BUILD_CLIENTS_ONLY "Build clients only" OFF)
option(BUILD_CONSUMER_TOOLS "Build consumer tools" OFF)
option(BUILD_EVENT_MONITOR_PRODUCER "Build the event monitor producer" OFF)

option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests" OFF)

option(BUILD_EXAMPLES "Build examples" OFF)
option(INSTALL_EXAMPLES "Install examples" OFF)

option(BUILD_CPP_DOCS "Build the C++ documentaion with doxygen" OFF)
option(BUILD_PYTHON_DOCS "Build the Python documentaion with sphinx" OFF)

option(ENABLE_LIBFABRIC "Enables LibFabric support for RDMA transfers" OFF)
option(ENABLE_LIBFABRIC_LOCALHOST "Emulates LibFabric stack over TCP. Only for localhost and testing purposes." OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/)

set (ASAPO_CXX_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/cpp/include)

find_package (Threads)

set (CMAKE_PREFIX_PATH "${LIBCURL_DIR}")
find_package (CURL REQUIRED)
message (STATUS "Found cURL libraries: ${CURL_LIBRARIES}")
message (STATUS "cURL include: ${CURL_INCLUDE_DIRS}")


if ("${Python_EXECUTABLE}" STREQUAL "")
    find_package (Python COMPONENTS Interpreter Development)
    if (NOT Python_FOUND)
        message (FATAL "Cannot find Python")
    endif()
endif()
message (STATUS "Using Python: ${Python_EXECUTABLE}")


SET(ASAPO_COMMON_FABRIC_LIBRARIES ${ASAPO_COMMON_IO_LIBRARIES})
IF(ENABLE_LIBFABRIC)
    find_package(LibFabric)
    if(NOT LIBFABRIC_LIBRARY)
        message(FATAL_ERROR "Did not find libfabric")
    endif()
    message(STATUS "LibFabric support enabled")
    message(STATUS "LIB_FABRIC: Path: ${LIBFABRIC_LIBRARY} Include: ${LIBFABRIC_INCLUDE_DIR}")
    add_definitions(-DLIBFABRIC_ENABLED)
    SET(ASAPO_COMMON_FABRIC_LIBRARIES ${ASAPO_COMMON_FABRIC_LIBRARIES} dl)
    IF(ENABLE_LIBFABRIC_LOCALHOST)
        message(STATUS "LIB_FABRIC: Enabled emulated RDMA when localhost is used. Should only be used for tests.")
        add_definitions(-DLIBFARBIC_ALLOW_LOCALHOST)
    ENDIF()
ENDIF()
SET_PROPERTY(GLOBAL PROPERTY ASAPO_COMMON_FABRIC_LIBRARIES ${ASAPO_COMMON_FABRIC_LIBRARIES})

# format sources
include(astyle)

include(prepare_version_tag)

include(testing_cpp)

include(prepare_asapo)

add_subdirectory(common/cpp)

add_subdirectory(producer)
add_subdirectory(consumer)

if (NOT BUILD_CLIENTS_ONLY)
add_subdirectory(broker)
add_subdirectory(receiver)
add_subdirectory(discovery)
add_subdirectory(authorizer)
add_subdirectory(asapo_tools)
add_subdirectory(file_transfer)
endif()

add_subdirectory(deploy)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES OR INSTALL_EXAMPLES)
    add_subdirectory(examples)
endif()

include(prepare_version)

add_subdirectory(docs)

if(NOT PACKAGE_RELEASE_SUFFIX)
    set(PACKAGE_RELEASE_SUFFIX linux)
endif()


set(CPACK_PACKAGE_VERSION ${ASAPO_VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "asapo")
set(CPACK_PACKAGE_RELEASE 1.${PACKAGE_RELEASE_SUFFIX})
set(CPACK_PACKAGE_CONTACT "IT")
set(CPACK_PACKAGE_VENDOR "DESY")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
