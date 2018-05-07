set(TARGET_NAME content_multithread)
set(SOURCE_FILES content_multithread.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-worker ${ASAPO_COMMON_IO_LIBRARIES})
if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "--coverage")
endif()


################################
# Testing
################################

add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} connected "test 10")

