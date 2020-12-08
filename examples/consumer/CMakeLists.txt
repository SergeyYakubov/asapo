find_package(Threads)

add_subdirectory(getnext_broker)

if(BUILD_EXAMPLES)
    if (UNIX OR CMAKE_BUILD_TYPE STREQUAL "Release")
        add_subdirectory(getnext_broker_python)
    endif()
endif()


