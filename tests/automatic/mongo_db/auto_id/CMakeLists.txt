set(TARGET_NAME mongo-auto-id)
set(SOURCE_FILES auto_id.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common database)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################
add_test_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} trans "trans 4 100" "1")
add_integration_test(${TARGET_NAME} seq "seq 4 100" "1")
