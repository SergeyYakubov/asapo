set(TARGET_NAME next_multithread_broker)
set(SOURCE_FILES next_multithread_broker.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common hidra2-worker)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json settings.json COPYONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}> $<TARGET_PROPERTY:hidra2-broker,EXENAME>"
        )

