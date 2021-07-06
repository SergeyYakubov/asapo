set(TARGET_NAME receiver_cpu_usage)

################################
# Testing
################################
set (ROOT_PATH "/tmp/asapo/")

configure_file(test.json.in test.json @ONLY)

add_script_test("${TARGET_NAME}" "$<TARGET_FILE:event-monitor-producer-bin> $<TARGET_FILE:getnext> $<TARGET_PROPERTY:asapo,EXENAME>" nomem)
