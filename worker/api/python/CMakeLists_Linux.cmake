if ((CMAKE_BUILD_TYPE STREQUAL "Debug") AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    set (EXTRA_COMPILE_ARGS "['--std=c++11']")
    set (EXTRA_LINK_ARGS "['--coverage','-fprofile-arcs','-ftest-coverage','-static-libgcc','-static-libstdc++']")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (EXTRA_COMPILE_ARGS "['--std=c++11']")
    set (EXTRA_LINK_ARGS "['-static-libgcc','-static-libstdc++']")
else()
    set (EXTRA_COMPILE_ARGS "['-std=c++11']")
    set (EXTRA_LINK_ARGS "[]")
ENDIF()

get_property(ASAPO_WORKER_LIB TARGET asapo-worker PROPERTY LOCATION)

set (ASAPO_WORKER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include)

configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

ADD_CUSTOM_TARGET(python-lib2 ALL
        COMMAND python setup.py build_ext --inplace)

ADD_CUSTOM_TARGET(python-lib3 ALL
        COMMAND python3 setup.py build_ext --inplace)

ADD_DEPENDENCIES(python-lib2 asapo-worker)
ADD_DEPENDENCIES(python-lib3 asapo-worker)

add_subdirectory(source_dist_linux)