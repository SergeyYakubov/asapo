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


add_subdirectory(worker)


add_subdirectory(producer_receiver)