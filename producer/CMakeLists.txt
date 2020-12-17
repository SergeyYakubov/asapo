add_subdirectory(api/cpp)

if (BUILD_PYTHON)
    add_subdirectory(api/python)
endif()

if(BUILD_EVENT_MONITOR_PRODUCER)
    add_subdirectory(event_monitor_producer)
endif(BUILD_EVENT_MONITOR_PRODUCER)
