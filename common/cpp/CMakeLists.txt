set(TARGET_NAME common)
set(SOURCE_FILES
        include/common/os.h
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
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})
#add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
