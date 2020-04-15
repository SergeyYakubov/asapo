set(TARGET_NAME read_folder_content)
set(SOURCE_FILES read_folder_content.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES}  $<TARGET_OBJECTS:system_io>)
target_link_libraries(${TARGET_NAME} test_common)

#Add all necessary common libraries
GET_PROPERTY(ASAPO_COMMON_IO_LIBRARIES GLOBAL PROPERTY ASAPO_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${ASAPO_COMMON_IO_LIBRARIES})

target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
IF(WIN32)
    add_integration_test(${TARGET_NAME} create_list "test 23subtest\\subtest2\\41")
ELSE()
    add_integration_test(${TARGET_NAME} create_list "test 23subtest/subtest2/41")
ENDIF(WIN32)


add_integration_test(${TARGET_NAME} foldernotfound "test_notexist Nosuchfileordirectory:test_notexist")
add_integration_test(${TARGET_NAME} foldernoaccess "test_noaccess1 Permissiondenied:test_noaccess1")

