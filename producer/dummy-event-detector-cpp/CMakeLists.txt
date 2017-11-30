set(TARGET_NAME dummy-event-detector-cpp)
set(SOURCE_FILES
        src/main.cpp
        src/dummy_yieldable.h src/dummy_yieldable.cpp
        src/dummy_detector.h src/dummy_detector.cpp
        )


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE include)
target_link_libraries(${TARGET_NAME} pthread common producer-api)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

astyle(${TARGET_NAME} "${SOURCE_FILES}")


################################
# Testing
################################
set(TEST_SOURCE_FILES unittests/test_dummy_detector.cpp)

gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} "")
