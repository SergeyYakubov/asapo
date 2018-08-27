set(TARGET_NAME file-monitor-producer)

################################
# Testing
################################
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:event-monitor-producer-bin>" nomem)
