add_subdirectory(file_monitor_producer)
add_subdirectory(beamtime_metadata)

add_subdirectory(cpp_api)
add_subdirectory(c_api)

if (BUILD_PYTHON)
    add_subdirectory(python_api)
    add_subdirectory(aai)
endif()
