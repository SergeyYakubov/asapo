if(BUILD_PYTHON)
    set(BUILD_PYTHON_PACKAGES "" CACHE STRING "which python packages to build")
    set_property(CACHE BUILD_PYTHON_PACKAGES PROPERTY STRINGS source rpm deb win)
endif()

set (CMAKE_PREFIX_PATH "${LIBCURL_DIR}")
find_package (CURL REQUIRED)
message (STATUS "Found cURL libraries: ${CURL_LIBRARIES}")
message (STATUS "cURL include: ${CURL_INCLUDE_DIRS}")
if(CURL_FOUND) #old FindCURL versions do not create CURL::libcurl target, so we do it here if CURL::libcurl is missing
    if(NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl UNKNOWN IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
        set_target_properties(CURL::libcurl PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                IMPORTED_LOCATION "${CURL_LIBRARIES}")
    endif()
endif()


find_package(RdKafka REQUIRED)
message (STATUS "Found rdkafka++ libraries: ${RDKAFKA_LIBRARIES}")
message (STATUS "rdkafka++ include dir : ${RDKAFKA_INCLUDE_DIR}")
if (WIN32)
    message (STATUS "rdkafka++ binary dir (dll): ${RDKAFKA_BIN_DIR}")
endif()

# python is needed anyway, even if no Python packages are build (e.g. to parse test results)
if ("${Python_EXECUTABLE}" STREQUAL "")
    find_package (Python COMPONENTS Interpreter Development)
    if (NOT Python_FOUND)
        message (FATAL "Cannot find Python")
    endif()
endif()
message (STATUS "Using Python: ${Python_EXECUTABLE}")

include(libfabric)