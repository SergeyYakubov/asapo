CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures
find_package(Threads)

add_subdirectory(common/cpp)
add_subdirectory(system_io)

add_subdirectory(json_parser)

if(BUILD_MONGODB_CLIENTLIB)
    add_subdirectory(mongo_db)
endif()


if(BUILD_BROKER)
    add_subdirectory(broker)
endif()

add_subdirectory(authorizer)

add_subdirectory(worker)

add_subdirectory(curl_http_client)

add_subdirectory(producer_receiver)

add_subdirectory(full_chain)

add_subdirectory(spd_logger)

add_subdirectory(producer)

if (UNIX)
    add_subdirectory(high_avail)
endif()