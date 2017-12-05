set(TARGET_NAME read_folder_content)
set(SOURCE_FILES read_folder_content.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} common)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test(NAME test-${TARGET_NAME} COMMAND ${TARGET_NAME} test notfound)
set_tests_properties(test-${TARGET_NAME} PROPERTIES LABELS "integration;all")
