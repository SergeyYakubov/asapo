set(TARGET_NAME common)
set(SOURCE_FILES include/common/os.h include/common/networking.h include/system_wrappers/io.h
        include/system_wrappers/system_io.h src/system_io.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

astyle(${TARGET_NAME} "${SOURCE_FILES}")


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_common.cpp)
set(TEST_LIBRARIES common)
gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} ${TEST_LIBRARIES})
