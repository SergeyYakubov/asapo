set(TARGET_NAME check-mongo-restart)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer>" nomem)
