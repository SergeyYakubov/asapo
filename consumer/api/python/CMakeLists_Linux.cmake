
set (TEMP_ADDITIONAL_LINK_ARGS_PART "") # Arguments for the linker

if ((CMAKE_BUILD_TYPE STREQUAL "Debug") AND (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    set (EXTRA_COMPILE_ARGS "['--std=c++11','-Wno-maybe-uninitialized']")
    set (EXTRA_LINK_ARGS "['--coverage','-fprofile-arcs','-ftest-coverage','-static-libgcc','-static-libstdc++' ${TEMP_ADDITIONAL_LINK_ARGS_PART}]")
ELSEIF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (EXTRA_COMPILE_ARGS "['--std=c++11','-Wno-maybe-uninitialized']")
    set (EXTRA_LINK_ARGS "['-static-libgcc','-static-libstdc++','-Wl,--exclude-libs,ALL' ${TEMP_ADDITIONAL_LINK_ARGS_PART}]")
else()
    set (EXTRA_COMPILE_ARGS "['-std=c++11']")
    set (EXTRA_LINK_ARGS "['' ${TEMP_ADDITIONAL_LINK_ARGS_PART}]")
ENDIF()

configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py INPUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

set (ASAPO_CONSUMER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include)

configure_files(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} @ONLY)

ADD_CUSTOM_TARGET(python-lib-consumer ALL COMMAND ${Python_EXECUTABLE} setup.py --quiet build_ext --inplace --force)

ADD_DEPENDENCIES(python-lib-consumer asapo-consumer)

if (BUILD_PYTHON_PACKAGES)
    add_subdirectory(dist_linux)
endif()
