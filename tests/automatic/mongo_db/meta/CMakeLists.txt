set(TARGET_NAME meta_mongodb)
set(SOURCE_FILES meta_mongodb.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common database)
target_include_directories(${TARGET_NAME} PUBLIC ${ASAPO_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################
add_integration_test(${TARGET_NAME} metaOK "OK" "OK")
add_integration_test(${TARGET_NAME} metaFailsWhenNotConnected
        "Notconnected"
        "Notconnected")
add_integration_test(${TARGET_NAME} metaFailsOnWrongMeta "parseerror" "parseerror")
add_test_cleanup(${TARGET_NAME})
