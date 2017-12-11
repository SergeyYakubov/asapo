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

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} create_list "test 23subtest/subtest241")
add_integration_test(${TARGET_NAME} foldernotfound "test_notexist notfound")
add_integration_test(${TARGET_NAME} foldernoaccess "test_noaccess noaccess")

