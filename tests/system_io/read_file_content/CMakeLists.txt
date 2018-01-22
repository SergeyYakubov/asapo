set(TARGET_NAME read_file_content)
set(SOURCE_FILES read_file_content.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES} $<TARGET_OBJECTS:common> )
target_link_libraries(${TARGET_NAME} test_common Threads::Threads)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/common/cpp/include)
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} readfile "test/1 123")
add_integration_test(${TARGET_NAME} filenotfound "test_notexist notfound")
add_integration_test(${TARGET_NAME} filenoaccess "file_noaccess noaccess")

