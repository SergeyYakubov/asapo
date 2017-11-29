set(TARGET_NAME inotify-event-detector-cpp)
set(SOURCE_FILES
        src/main.cpp
        src/inotify_helper.cpp include/inotify-event-detector-cpp/inotify_helper.h
        src/Inotify_event_detector.cpp include/inotify-event-detector-cpp/Inotify_event_detector.h
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
set(TEST_SOURCE_FILES unittests/test_inotify_client.cpp)
gtest(${TARGET_NAME} ${TEST_SOURCE_FILES} "")
