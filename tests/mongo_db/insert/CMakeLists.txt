set(TARGET_NAME insert_mongodb)
set(SOURCE_FILES insert_mongodb.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common mongo_db)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################
add_test_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} insertOK "OK 1" "OK 2")
add_integration_test(${TARGET_NAME} insertFailsWhenNotConnected
        "NotConnected 3"
        "NotConnected 4")
add_integration_test(${TARGET_NAME} insertFailsForDuplicateID
        "DuplicateID 6"
        "DuplicateID 5")


