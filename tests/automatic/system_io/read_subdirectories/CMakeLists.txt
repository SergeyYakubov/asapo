set(TARGET_NAME read_subdirectories)
set(SOURCE_FILES read_subdirectories.cpp)


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
    add_integration_test(${TARGET_NAME} list_folders "test test\\subtest1test\\subtest1\\subtest2test\\subtest3test\\subtest3\\subtest4")
ELSE()
    add_integration_test(${TARGET_NAME} list_folders "test test/subtest1test/subtest1/subtest2test")
ENDIF(WIN32)


add_integration_test(${TARGET_NAME} foldernotfound "test_notexist Nosuchfileordirectory:test_notexist")
add_integration_test(${TARGET_NAME} foldernoaccess "test_noaccess1 Permissiondenied:test_noaccess1")

