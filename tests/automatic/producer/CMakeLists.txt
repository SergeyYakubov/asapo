add_subdirectory(file_monitor_producer)
add_subdirectory(beamtime_metadata)

if (UNIX OR CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(python_api)
    add_subdirectory(aai)

endif()
