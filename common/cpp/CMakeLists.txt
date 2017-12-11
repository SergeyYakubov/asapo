set(TARGET_NAME common)
set(SOURCE_FILES
        src/io_utils.cpp
        src/has_io.cpp
        src/system_io.cpp)
IF(WIN32)
    # Do windows specific includes
ELSEIF(UNIX)
    set(SOURCE_FILES ${SOURCE_FILES} src/system_io_linux.cpp)
ENDIF(WIN32)


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
