
if (UNIX)
    include(CMakeLists_Linux.cmake)
    set (suf "so")
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    set (suf "pyd")
    include(CMakeLists_Windows.cmake)
ENDIF()