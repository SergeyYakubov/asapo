add_subdirectory(api/cpp)
add_subdirectory(api/python)

if(BUILD_EVENT_MONITOR_PRODUCER)
    add_subdirectory(event_monitor_producer)
endif(BUILD_EVENT_MONITOR_PRODUCER)
