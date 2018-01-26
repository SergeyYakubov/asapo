
set(TARGET_NAME common)
set(SOURCE_FILES
        src/has_io.cpp
        include/common/networking.h
        include/system_wrappers/io.h
        include/system_wrappers/system_io.h
        src/system_io.cpp)
IF(WIN32)
    set(SOURCE_FILES ${SOURCE_FILES} src/system_io_windows.cpp)
ELSEIF(UNIX)
    set(SOURCE_FILES ${SOURCE_FILES} src/system_io_linux.cpp)
ENDIF(WIN32)


################################
# Library
################################
IF(WIN32)
    find_package(Threads REQUIRED)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} wsock32 ws2_32)
ELSEIF(UNIX)
    SET_PROPERTY(GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES Threads::Threads)
ENDIF(WIN32)
add_library(${TARGET_NAME} OBJECT ${SOURCE_FILES})
#add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

install(DIRECTORY include/ DESTINATION include)
