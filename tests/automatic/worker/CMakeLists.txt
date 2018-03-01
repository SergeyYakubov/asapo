CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures

add_subdirectory(next_multithread_folder)
add_subdirectory(next_multithread_broker)
add_subdirectory(connect_multithread)

add_subdirectory(curl_http_client_get)

if(BUILD_WORKER_TOOLS)
    add_subdirectory(folder_to_db)
endif()


