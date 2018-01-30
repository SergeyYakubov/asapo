add_subdirectory(api/cpp)

if(BUILD_WORKER_TOOLS)
    set (BUILD_MONGODB ON)
    add_subdirectory(tools)
endif()
