set(TARGET_NAME upsert_mongodb)
set(SOURCE_FILES upsert_mongodb.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common database)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################
add_integration_test(${TARGET_NAME} upsertOK "OK" "OK")
add_integration_test(${TARGET_NAME} upsertFailsWhenNotConnected
        "Notconnected"
        "Notconnected")
add_integration_test(${TARGET_NAME} upsertFailsOnWrongMeta "parseerror" "parseerror")
add_test_cleanup(${TARGET_NAME})
