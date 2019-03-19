
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
        PATTERN "binary_dist_windows" EXCLUDE
        PATTERN "CMakeFiles" EXCLUDE
        )

