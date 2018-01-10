set(TARGET_NAME upsert_mongodb)
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
add_test_setup_cleanup(${TARGET_NAME})
add_integration_test(${TARGET_NAME} upsertOK "OK 1" "OK 2")
add_integration_test(${TARGET_NAME} upsertFailsWhenNotConnected
        "NotConnected 3"
        "NotConnected 4")
add_integration_test(${TARGET_NAME} upsertFailsForDuplicateID
        "DuplicateID 6"
        "DuplicateID 5")


