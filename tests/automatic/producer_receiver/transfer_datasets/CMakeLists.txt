set(TARGET_NAME transfer-datasets)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
