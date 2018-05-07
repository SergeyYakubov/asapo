set(TARGET_NAME transfer-single-file)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin>" nomem)
