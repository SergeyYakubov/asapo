set(TARGET_NAME system_io)
set(SOURCE_FILES
        io_factory.cpp
        system_io.cpp)
IF(WIN32)
    set(SOURCE_FILES ${SOURCE_FILES} system_io_windows.cpp)
ELSEIF(UNIX AND APPLE)
    set(SOURCE_FILES ${SOURCE_FILES} system_io_linux_mac.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} system_io_mac.cpp)
ELSEIF(UNIX)
    set(SOURCE_FILES ${SOURCE_FILES} system_io_linux_mac.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} system_io_linux.cpp)
ENDIF(WIN32)


################################
# Library
################################
add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})


