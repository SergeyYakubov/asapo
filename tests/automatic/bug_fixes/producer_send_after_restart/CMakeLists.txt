set(TARGET_NAME producer_send_after_restart)

################################
# Testing
################################
prepare_asapo()

if (UNIX)
    set (ROOT_PATH "/tmp/asapo/")
else()
    set (ROOT_PATH "c:\\\\tmp\\\\asapo\\\\")
endif()

configure_file(test.json.in test.json @ONLY)

add_script_test("${TARGET_NAME}-tcp" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
