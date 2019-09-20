CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures

add_subdirectory(next_multithread_broker)
add_subdirectory(worker_api)

if (UNIX OR CMAKE_BUILD_TYPE STREQUAL "Release")
    add_subdirectory(worker_api_python)
endif()

if(BUILD_WORKER_TOOLS)
    add_subdirectory(folder_to_db)
endif()


