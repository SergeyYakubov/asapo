set(TARGET_NAME broker_mongo_restart)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)
