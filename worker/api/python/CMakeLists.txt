
get_property(ASAPO_WORKER_LIB TARGET asapo-worker PROPERTY LOCATION)

set (ASAPO_WORKER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/include)

if (UNIX)
    include(CMakeLists_Linux.cmake)
    set (suf "so")
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (suf "pyd")
    include(CMakeLists_Windows.cmake)
ENDIF()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ DESTINATION lib/python/
        FILES_MATCHING PATTERN "*.${suf}"
        PATTERN "build" EXCLUDE
        PATTERN "source_dist_linux" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        )

