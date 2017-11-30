set(TARGET_NAME receiver)
set(SOURCE_FILES
        src/main.cpp
        src/receiver.h src/receiver.cpp)


################################
# Library
################################
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include)
target_link_libraries(${TARGET_NAME} common)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

astyle(${TARGET_NAME} "${SOURCE_FILES}")


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_receiver.cpp)
set(TEST_LIBRARIES producer-api)


gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} ${TEST_LIBRARIES})


