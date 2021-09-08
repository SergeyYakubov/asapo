cmake_minimum_required(VERSION 3.7)
project(ASAPO)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules/ ${PROJECT_SOURCE_DIR}/CMakeIncludes/)

set (ASAPO_VERSION_IN_DOCS 21.08.0) # is overwritten in master branch
set (ASAPO_EXAMPLES_DIR .)
#set (ASAPO_EXAMPLES_DIR ./frozen_versions/${ASAPO_VERSION_IN_DOCS})

#protocol version changes if one of the microservice API's change
set (ASAPO_CONSUMER_PROTOCOL "v0.4")
set (ASAPO_PRODUCER_PROTOCOL "v0.3")
set (ASAPO_DISCOVERY_API_VER "v0.1")
set (ASAPO_AUTHORIZER_API_VER "v0.2")
set (ASAPO_BROKER_API_VER "v0.4")
set (ASAPO_FILE_TRANSFER_SERVICE_API_VER "v0.2")
set (ASAPO_RECEIVER_API_VER "v0.3")
set (ASAPO_RDS_API_VER "v0.1")
set (DB_SCHEMA_VER "v0.1")

#global settings

set(ConfigPackageLocation lib/cmake/Asapo)


# options

option(BUILD_PYTHON "Build python libs" ON)
option(BUILD_CLIENTS_ONLY "Build clients only" OFF)

option(BUILD_CONSUMER_TOOLS "Build consumer tools" OFF)
option(BUILD_EVENT_MONITOR_PRODUCER "Build the event monitor producer" OFF)

option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_INTEGRATION_TESTS "Include integration tests" OFF)

option(BUILD_EXAMPLES "Build examples" OFF)
option(INSTALL_EXAMPLES "Install examples" OFF)

option(STOP_BUILD_ON_WARNINGS "Stop build if compiler has warnings" OFF)

option(BUILD_CPP_DOCS "Build the C++ documentaion with doxygen" OFF)
option(BUILD_PYTHON_DOCS "Build the Python documentaion with sphinx" OFF)
option(BUILD_ASAPO_SITE "Build the asapo web site" OFF)

option(ENABLE_LIBFABRIC "Enables LibFabric support for RDMA transfers" OFF)
option(ENABLE_LIBFABRIC_LOCALHOST "Emulates LibFabric stack over TCP. Only for localhost and testing purposes." OFF)

option(BUILD_SHARED_CLIENT_LIBS "Build shared consumer and producer libs" ON)
option(BUILD_STATIC_CLIENT_LIBS "Build static consumer and producer libs" ON)

# includes

include(language_settings)
include(dependencies)

include(astyle)

include(prepare_version_tag)
include(testing_cpp)
include(prepare_asapo)
include(prepare_version)
include(packaging)

#subdirectorties

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
    add_subdirectory(monitoring)
endif()

add_subdirectory(deploy)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES OR INSTALL_EXAMPLES OR BUILD_ASAPO_SITE)
    add_subdirectory(examples)
endif()

add_subdirectory(docs)

add_subdirectory(install)

