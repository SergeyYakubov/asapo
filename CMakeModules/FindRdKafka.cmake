# FindRdKafka
# -------------
#
# Tries to find RdKafka on the system
#
# Available variables
# RDKAFKA_LIBRARIES         - Path to the library
# RDKAFKA_INCLUDE_DIR     - Path to the include dir

cmake_minimum_required(VERSION 3.12)

find_path(RDKAFKA_INCLUDE_DIR librdkafka/rdkafka.h HINTS ${RdKafka_DIR}/include)
find_library(RDKAFKA_LIBRARIES rdkafka++ HINTS ${RdKafka_DIR}/lib ${RdKafka_DIR}/lib64)

mark_as_advanced(RDKAFKA_LIBRARIES RDKAFKA_INCLUDE_DIR)
