set(TARGET_NAME consumer_api)
set(SOURCE_FILES consumer_api.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-consumer)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")

