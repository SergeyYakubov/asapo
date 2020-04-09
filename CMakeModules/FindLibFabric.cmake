# FindLibFabric
# -------------
#
# Tries to find LibFabric on the system
#
# Available variables
# LIBFABRIC_LIBRARY         - Path to the library
# LIBFABRIC_INCLUDE_DIR     - Path to the include dir

cmake_minimum_required(VERSION 2.6)

find_path(LIBFABRIC_INCLUDE_DIR fabric.h)
find_library(LIBFABRIC_LIBRARY fabric)

mark_as_advanced(LIBFABRIC_INCLUDE_DIR LIBFABRIC_LIBRARY)
