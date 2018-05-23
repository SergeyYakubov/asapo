set(TARGET_NAME full_chain_simple_chain)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin> $<TARGET_PROPERTY:asapo-broker,EXENAME> $<TARGET_FILE:getnext_broker>" nomem)
