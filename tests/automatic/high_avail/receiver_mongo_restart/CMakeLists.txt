set(TARGET_NAME receiver-mongo-restart)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/admin_token.key admin_token.key COPYONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
