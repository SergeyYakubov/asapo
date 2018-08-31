set(TARGET_NAME file-monitor-producer)

################################
# Testing
################################
configure_file(test.json test.json COPYONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:event-monitor-producer-bin>" nomem)
