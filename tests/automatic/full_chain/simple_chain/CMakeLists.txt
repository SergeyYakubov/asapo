set(TARGET_NAME full_chain_simple_chain)

################################
# Testing
################################
prepare_asapo()
add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME> tcp" nomem)

if (ENABLE_LIBFABRIC)
    add_script_test("${TARGET_NAME}-fabric" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:getnext_broker> $<TARGET_PROPERTY:asapo,EXENAME> fabric" nomem)
endif()
