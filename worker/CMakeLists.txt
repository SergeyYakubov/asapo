add_subdirectory(api/cpp)
IF (UNIX)
 add_subdirectory(api/python)
ENDIF()

if(BUILD_WORKER_TOOLS)
    set (BUILD_MONGODB ON)
    add_subdirectory(tools)
endif()
