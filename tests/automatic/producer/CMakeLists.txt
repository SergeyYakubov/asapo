add_subdirectory(file_monitor_producer)
add_subdirectory(beamtime_metadata)

if (BUILD_PYTHON)
    add_subdirectory(python_api)
    add_subdirectory(aai)
endif()
