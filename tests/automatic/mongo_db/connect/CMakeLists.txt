set(TARGET_NAME connect_mongodb)
set(SOURCE_FILES connect_mongodb.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common database)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################

add_integration_test(${TARGET_NAME} connectOK "127.0.0.1 data test OK")
add_integration_test(${TARGET_NAME} connectFAILS "127.0.0.0 data test Connectionerror")
add_integration_test(${TARGET_NAME} connectBadAddress "#?ß// data test Badaddress")


