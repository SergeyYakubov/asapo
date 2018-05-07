set(TARGET_NAME parse_config_file)
set(SOURCE_FILES parse_config_file.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common asapo-worker)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################

add_integration_test(${TARGET_NAME} parseOK "${CMAKE_CURRENT_SOURCE_DIR}/config.json OK")
add_integration_test(${TARGET_NAME} parseFails "${CMAKE_CURRENT_SOURCE_DIR}/config_bad.json FAIL")


