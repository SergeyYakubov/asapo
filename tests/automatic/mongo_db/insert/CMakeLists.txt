set(TARGET_NAME insert_mongodb)
set(SOURCE_FILES insert_mongodb.cpp)


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
add_integration_test(${TARGET_NAME} insertOK "OK 1" "OK 2")
add_integration_test(${TARGET_NAME} insertFailsWhenNotConnected
        "Notconnected 3"
        "Notconnected 4")
add_integration_test(${TARGET_NAME} insertFailsForDuplicateID
        "DuplicateID 6"
        "DuplicateID 5")


