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

add_test(NAME test-${TARGET_NAME}-create_list COMMAND ${TARGET_NAME} test 23subtest/subtest241)
add_test(NAME test-${TARGET_NAME}-foldernotfound COMMAND ${TARGET_NAME} test_notexist notfound)
add_test(NAME test-${TARGET_NAME}-foldernoaccess COMMAND ${TARGET_NAME} test_noaccess noaccess)


set_tests_properties(test-${TARGET_NAME}-create_list PROPERTIES
        LABELS "integration;all"
        FIXTURES_REQUIRED ${TARGET_NAME}-fixture
        )

set_tests_properties(test-${TARGET_NAME}-foldernotfound PROPERTIES
        LABELS "integration;all"
#        FIXTURES_REQUIRED ${TARGET_NAME}-fixture
        )


set_tests_properties(test-${TARGET_NAME}-foldernoaccess PROPERTIES
        LABELS "integration;all"
        FIXTURES_REQUIRED ${TARGET_NAME}-fixture
        )


add_test(NAME test-${TARGET_NAME}-setup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/setup.sh)
add_test(NAME test-${TARGET_NAME}-cleanup COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/cleanup.sh)
set_tests_properties(test-${TARGET_NAME}-setup    PROPERTIES FIXTURES_SETUP   ${TARGET_NAME}-fixture)
set_tests_properties(test-${TARGET_NAME}-cleanup  PROPERTIES FIXTURES_CLEANUP ${TARGET_NAME}-fixture)




