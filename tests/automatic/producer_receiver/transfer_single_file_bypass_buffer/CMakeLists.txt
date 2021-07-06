set(TARGET_NAME transfer-single-file_bypass)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
