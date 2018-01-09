CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures
find_package(Threads)

add_subdirectory(common/cpp)
add_subdirectory(system_io)

if(BUILD_MONGODB_CLIENTLIB)
    add_subdirectory(mongo_db)
endif()

add_subdirectory(worker)


