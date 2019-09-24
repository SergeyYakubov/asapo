add_subdirectory(api/cpp)
add_subdirectory(api/python)


if(BUILD_CONSUMER_TOOLS)
    set (BUILD_MONGODB ON)
    add_subdirectory(tools)
endif()
