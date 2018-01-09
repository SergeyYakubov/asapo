CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures

add_subdirectory(next_multithread)
add_subdirectory(connect_multithread)
if(BUILD_WORKER_TOOLS)
    add_subdirectory(folder_to_db)
endif()


