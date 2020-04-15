set(TARGET_NAME next_multithread_broker)
set(SOURCE_FILES next_multithread_broker.cpp)

################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-consumer)

################################
# Testing
################################

if (UNIX)
    prepare_asapo()
    add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")
endif()



