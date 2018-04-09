set(TARGET_NAME next_multithread_folder)
set(SOURCE_FILES next_multithread_folder.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})

#Add all necessary common libraries
GET_PROPERTY(HIDRA2_COMMON_IO_LIBRARIES GLOBAL PROPERTY HIDRA2_COMMON_IO_LIBRARIES)
target_link_libraries(${TARGET_NAME} ${HIDRA2_COMMON_IO_LIBRARIES})

target_link_libraries(${TARGET_NAME} test_common hidra2-worker)
if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()


################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} readnext "test 50 500" "test 50 1")

