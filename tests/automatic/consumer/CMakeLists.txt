CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures

add_subdirectory(next_multithread_broker)
add_subdirectory(consumer_api)

if (BUILD_PYTHON)
    add_subdirectory(consumer_api_python)
endif()

if(BUILD_CONSUMER_TOOLS)
    add_subdirectory(folder_to_db)
endif()


