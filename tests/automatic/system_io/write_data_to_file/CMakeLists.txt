set(TARGET_NAME write_data_to_file)
set(SOURCE_FILES write_data_to_file.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> )
target_link_libraries(${TARGET_NAME} test_common)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/common/cpp/include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################
add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} writeok "test_file ok dummy" nomem)
if (UNIX)
    add_integration_test(${TARGET_NAME} writewithfolder "folder/a/b/c/d/test_file ok dummy" nomem)
else ()
    add_integration_test(${TARGET_NAME} writewithfolder "folder\\a\\b\\c\\d\\test_file ok dummy" nomem)
endif()

add_integration_test(${TARGET_NAME} writetwice "test_file ok dummy" nomem)
add_integration_test(${TARGET_NAME} dirnoaccess "test_noaccess/test_file error error:permissiondenied,details:name:test_noaccess/test_file" nomem)

