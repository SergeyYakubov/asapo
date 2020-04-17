set(TARGET_NAME service_restart)

################################
# Testing
################################
set(RECEIVER_WRITE_TO_DISK false)
prepare_asapo()
add_script_test("${TARGET_NAME}-all" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME> broker 1000 998" nomem)
add_script_test("${TARGET_NAME}-all-but-broker" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME> receiver_tcp 1000 1000" nomem)
