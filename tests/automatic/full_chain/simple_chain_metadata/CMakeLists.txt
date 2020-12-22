set(TARGET_NAME full_chain_simple_chain_metadata)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)
