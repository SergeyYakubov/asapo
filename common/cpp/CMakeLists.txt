set(TARGET_NAME common)
set(SOURCE_FILES
        src/has_io.cpp
        src/io_utils.cpp
        src/system_io.cpp
        )


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
