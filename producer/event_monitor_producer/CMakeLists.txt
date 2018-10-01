set(TARGET_NAME event-monitor-producer)
set(SOURCE_FILES
        src/eventmon_config.cpp
        src/eventmon_logger.cpp
        src/event_detector_factory.cpp
        src/folder_event_detector.cpp
)
IF (APPLE)
    #do nothing
ELSEIF(WIN32)
    set(SOURCE_FILES ${SOURCE_FILES} src/system_folder_watch_windows.cpp
            src/single_folder_watch_windows.cpp
            src/watch_io.cpp
            src/shared_event_list.cpp
            src/win_event.cpp
            )
ELSEIF(UNIX)
    set(SOURCE_FILES ${SOURCE_FILES} src/system_folder_watch_linux.cpp src/inotify_event.cpp src/inotify_linux.cpp)
ENDIF(APPLE)



################################
# Library (needed for unit tests)
################################

add_library(${TARGET_NAME} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC include ${CMAKE_SOURCE_DIR}/common/cpp/include)

#Add all necessary common libraries
GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} producer-api)

################################
# Executable
################################
add_executable(${TARGET_NAME}-bin src/main_eventmon.cpp)
set_target_properties(${TARGET_NAME}-bin PROPERTIES OUTPUT_NAME asapo-eventmon-producer)
target_link_libraries(${TARGET_NAME}-bin ${TARGET_NAME})

set_target_properties(${TARGET_NAME}-bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}$<$<CONFIG:Debug>:>
        )


################################
# Testing
################################
set_property(TARGET ${TARGET_NAME} PROPERTY ENABLE_EXPORTS true)

set(TEST_SOURCE_FILES
        unittests/test_eventmon_config.cpp
        unittests/mock_eventmon_config.cpp
        unittests/test_event_detector_factory.cpp
        unittests/test_folder_event_detector.cpp
   )
IF (APPLE)
    #do nothing
ELSEIF(UNIX)
set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} unittests/test_system_folder_watch_linux.cpp)
ELSE()
set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} unittests/test_system_folder_watch_windows.cpp
        unittests/test_single_folder_watch_windows.cpp)
ENDIF(APPLE)


set(TEST_LIBRARIES "${TARGET_NAME}")

gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/main_eventmon.cpp)

install(TARGETS ${TARGET_NAME}-bin DESTINATION bin)

