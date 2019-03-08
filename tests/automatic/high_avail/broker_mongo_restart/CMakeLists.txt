set(TARGET_NAME broker_mongo_restart)

################################
# Testing
################################
set(RECEIVER_WRITE_TO_DISK false)
prepare_asapo()
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
