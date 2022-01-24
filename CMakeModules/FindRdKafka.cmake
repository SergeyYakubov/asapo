# FindRdKafka
# -------------
#
# Tries to find RdKafka on the system
#
# Available variables
# RDKAFKA_LIBRARIES         - Path to the library
# RDKAFKA_INCLUDE_DIR     - Path to the include dir

cmake_minimum_required(VERSION 3.7)

find_path(RDKAFKA_INCLUDE_DIR librdkafka/rdkafka.h HINTS ${RdKafka_DIR}/include)
find_library(RDKAFKA_LIBRARIES rdkafka++ HINTS ${RdKafka_DIR}/lib ${RdKafka_DIR}/lib64)
find_library(RDKAFKA_C_LIBRARIES rdkafka HINTS ${RdKafka_DIR}/lib ${RdKafka_DIR}/lib64)

IF(WIN32)
    find_path(RDKAFKA_BIN_DIR rdkafka++.dll HINTS ${RdKafka_DIR}/bin ${RdKafka_DIR}/lib)
    mark_as_advanced(RDKAFKA_BIN_DIR)
    find_package_handle_standard_args(RdKafka REQUIRED_VARS RDKAFKA_INCLUDE_DIR RDKAFKA_C_LIBRARIES RDKAFKA_LIBRARIES RDKAFKA_BIN_DIR)
ELSE()
    find_package_handle_standard_args(RdKafka REQUIRED_VARS RDKAFKA_INCLUDE_DIR RDKAFKA_C_LIBRARIES RDKAFKA_LIBRARIES)
ENDIF()

mark_as_advanced(RDKAFKA_LIBRARIES RDKAFKA_INCLUDE_DIR)
