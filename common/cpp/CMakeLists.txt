set(TARGET_NAME common)
set(SOURCE_FILES
        include/common/os.h
        include/common/networking.h
        include/system_wrappers/io.h
        include/system_wrappers/has_io.h src/has_io.cpp
        include/system_wrappers/io_utils.h src/io_utils.cpp
        include/system_wrappers/system_io.h src/system_io.cpp
        )


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
