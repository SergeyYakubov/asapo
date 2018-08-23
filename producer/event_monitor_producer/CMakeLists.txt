set(TARGET_NAME event-monitor-producer)
set(SOURCE_FILES
        src/event_detector.cpp
        src/eventmon_config.cpp
        src/eventmon_logger.cpp
        src/event_detector_factory.cpp
        src/inotify_event_detector.cpp)


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
add_executable(${TARGET_NAME}-bin src/eventmon_main.cpp)
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
   )

set(TEST_LIBRARIES "${TARGET_NAME}")

gtest(${TARGET_NAME} "${TEST_SOURCE_FILES}" "${TEST_LIBRARIES}" ${CMAKE_CURRENT_SOURCE_DIR}/src/eventmon_main.cpp)

install(TARGETS ${TARGET_NAME}-bin DESTINATION bin)

