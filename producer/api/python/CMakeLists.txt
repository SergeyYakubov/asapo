
if (UNIX)
    include(CMakeLists_Linux.cmake)
    set (suf "so")
ELSE()
    set (suf "pyd")
    include(CMakeLists_Windows.cmake)
ENDIF()