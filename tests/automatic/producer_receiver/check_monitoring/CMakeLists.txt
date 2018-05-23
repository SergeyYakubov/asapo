set(TARGET_NAME receiver)

################################
# Testing
################################
prepare_asapo()

add_script_test("${TARGET_NAME}-monitoring" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin> $<TARGET_PROPERTY:asapo-discovery,EXENAME>" nomem)
