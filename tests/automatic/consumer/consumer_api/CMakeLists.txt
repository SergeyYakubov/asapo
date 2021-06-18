set(TARGET_NAME consumer_api)
set(TARGET_NAME_C consumer_api_c)

set(SOURCE_FILES consumer_api.cpp)
set(SOURCE_FILES_C consumer_api.c)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
add_executable(${TARGET_NAME_C} ${SOURCE_FILES_C})

target_link_libraries(${TARGET_NAME} test_common asapo-consumer)
target_link_libraries(${TARGET_NAME_C} test_common asapo-consumer)


################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:${TARGET_NAME}>")
add_script_test("${TARGET_NAME_C}" "$<TARGET_FILE:${TARGET_NAME_C}>")



