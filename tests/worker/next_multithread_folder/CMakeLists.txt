set(TARGET_NAME next_multithread_folder)
set(SOURCE_FILES next_multithread_folder.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common hidra2-worker  ${CMAKE_THREAD_LIBS_INIT})

################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} readnext "test 50 500" "test 50 1")

