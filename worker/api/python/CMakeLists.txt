
get_property(ASAPO_WORKER_LIB TARGET asapo-worker PROPERTY LOCATION)
set (ASAPO_WORKER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include)

if ((CMAKE_BUILD_TYPE STREQUAL "Debug") AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    set (EXTRA_COMPILE_ARGS "['--std=c++11','--coverage','-fprofile-arcs','-ftest-coverage']")
    set (EXTRA_LINK_ARGS "['--coverage','-fprofile-arcs','-ftest-coverage','-static-libgcc','-static-libstdc++']")
ELSE()
    set (EXTRA_COMPILE_ARGS "['--std=c++11']")
    set (EXTRA_LINK_ARGS "['-static-libgcc','-static-libstdc++']")
ENDIF()


configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

ADD_CUSTOM_TARGET(python-lib2 ALL
        COMMAND python setup.py build_ext --inplace)

ADD_CUSTOM_TARGET(python-lib3 ALL
        COMMAND python3 setup.py build_ext --inplace)

ADD_DEPENDENCIES(python-lib2 asapo-worker)
ADD_DEPENDENCIES(python-lib3 asapo-worker)

if (WIN32)
    set (suf "dll")
ELSE()
    set (suf "so")
ENDIF()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ DESTINATION lib/python/
        FILES_MATCHING PATTERN "*.${suf}"
        PATTERN "build" EXCLUDE
        PATTERN "dist" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        )

add_subdirectory(dist)