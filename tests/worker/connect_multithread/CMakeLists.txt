set(TARGET_NAME content_multithread)
set(SOURCE_FILES content_multithread.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common common worker-api ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} connected "test 10")

