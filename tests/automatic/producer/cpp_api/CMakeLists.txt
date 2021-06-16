set(TARGET_NAME producer_api)
set(SOURCE_FILES producer_api.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-producer)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")

