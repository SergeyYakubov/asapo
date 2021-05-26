set(TARGET_NAME full_chain_simple_chain_filegen_producer_batches)

################################
# Testing
################################
if (UNIX)
    set (ROOT_PATH "/tmp/asapo/")
else()
    set (ROOT_PATH "c:\\\\tmp\\\\asapo\\\\")
endif()

configure_file(test.json.in test.json @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/admin_token.key admin_token.key COPYONLY)

add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
