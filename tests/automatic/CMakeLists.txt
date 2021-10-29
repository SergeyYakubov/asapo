CMAKE_MINIMUM_REQUIRED(VERSION 3.7) # needed for fixtures
find_package(Threads)

add_subdirectory(common_scripts)

add_subdirectory(support/getnext)

add_subdirectory(common/cpp)

add_subdirectory(system_io)

add_subdirectory(json_parser)

add_subdirectory(mongo_db)

add_subdirectory(broker)

add_subdirectory(authorizer)

add_subdirectory(consumer)

add_subdirectory(curl_http_client)

add_subdirectory(producer_receiver)

add_subdirectory(full_chain)

add_subdirectory(spd_logger)

add_subdirectory(producer)

add_subdirectory(file_transfer_service)


if (UNIX)
    add_subdirectory(high_avail)
endif()

add_subdirectory(bug_fixes)

if (ENABLE_LIBFABRIC)
    if (ENABLE_LIBFABRIC_LOCALHOST)
        add_subdirectory(asapo_fabric)
    else ()
        message(WARNING "Disabled automated LibFabric tests because 'ENABLE_LIBFABRIC_LOCALHOST' is not enabled.")
    endif()
endif()
