set(TARGET_NAME full_chain_simple_chain_filegen_producer_multisource)

################################
# Testing
################################
prepare_asapo()
if (UNIX)
    set (ROOT_PATH "/tmp/asapo/")
else()
    set (ROOT_PATH "c:\\\\tmp\\\\asapo\\\\")
endif()

SET (ID 1)
configure_file(test.json.in test1.json @ONLY)
SET (ID 2)
configure_file(test.json.in test2.json @ONLY)

add_script_test("${TARGET_NAME}" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
