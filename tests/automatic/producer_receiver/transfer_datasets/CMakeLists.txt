set(TARGET_NAME transfer-datasets)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
