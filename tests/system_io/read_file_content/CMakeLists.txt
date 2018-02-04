set(TARGET_NAME read_file_content)
set(SOURCE_FILES read_file_content.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:system_io> )
target_link_libraries(${TARGET_NAME} test_common)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} readfile "test/1 123")
add_integration_test(${TARGET_NAME} filenotfound "test_notexist notfound")
add_integration_test(${TARGET_NAME} filenoaccess "file_noaccess noaccess")

