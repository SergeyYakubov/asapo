set(TARGET_NAME read_folder_content)
set(SOURCE_FILES read_folder_content.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES}  $<TARGET_OBJECTS:common>)
target_link_libraries(${TARGET_NAME} test_common Threads::Threads)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/common/cpp/include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
IF(WIN32)
    add_integration_test(${TARGET_NAME} create_list "test 23subtest\\subtest241")
ELSE()
    add_integration_test(${TARGET_NAME} create_list "test 23subtest/subtest241")
ENDIF(WIN32)


add_integration_test(${TARGET_NAME} foldernotfound "test_notexist notfound")
add_integration_test(${TARGET_NAME} foldernoaccess "test_noaccess1 noaccess")

