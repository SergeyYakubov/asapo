if ((CMAKE_BUILD_TYPE STREQUAL "Debug") AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    set (EXTRA_COMPILE_ARGS "['--std=c++11','-Wno-maybe-uninitialized']")
    set (EXTRA_LINK_ARGS "['--coverage','-fprofile-arcs','-ftest-coverage','-static-libgcc','-static-libstdc++']")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (EXTRA_COMPILE_ARGS "['--std=c++11','-Wno-maybe-uninitialized']")
    set (EXTRA_LINK_ARGS "['-static-libgcc','-static-libstdc++','-Wl,--exclude-libs,ALL']")
else()
    set (EXTRA_COMPILE_ARGS "['-std=c++11']")
    set (EXTRA_LINK_ARGS "[]")
ENDIF()

set (ASAPO_PRODUCER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include)

configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

ADD_CUSTOM_TARGET(python-lib-producer ALL
        COMMAND ${Python_EXECUTABLE} setup.py  --quiet build_ext --inplace --force)

ADD_DEPENDENCIES(python-lib-producer asapo-producer)

if (BUILD_PYTHON_PACKAGES)
    add_subdirectory(dist_linux)
endif()
