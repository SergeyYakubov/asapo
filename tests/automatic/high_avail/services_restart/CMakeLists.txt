set(TARGET_NAME service_restart)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}-all" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> broker 1000 998" nomem)
add_script_test("${TARGET_NAME}-all-but-broker" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> receiver 1000 1000" nomem)
