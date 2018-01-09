set(TARGET_NAME import_mongodb)
set(SOURCE_FILES import_mongodb.cpp)


################################
# Executable and link
################################
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} test_common mongo_db)
target_include_directories(${TARGET_NAME} PUBLIC ${HIDRA2_CXX_COMMON_INCLUDE_DIR})

################################
# Testing
################################

add_integration_test(${TARGET_NAME} importOK "ok OK")
add_integration_test(${TARGET_NAME} importFailsWhenNotConnected "notconnected NotConnected")


