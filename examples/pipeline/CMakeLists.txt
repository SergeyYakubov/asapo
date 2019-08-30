
add_subdirectory(in_to_out)

if (UNIX OR CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(in_to_out_python)
endif()
