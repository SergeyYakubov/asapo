set(TARGET_NAME file-monitor-producer)

################################
# Testing
################################
if (UNIX)
    set (ROOT_PATH "/tmp/asapo/")
else()
    set (ROOT_PATH "c:\\\\tmp\\\\asapo\\\\")
endif()

configure_file(test.json.in test.json @ONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:event-monitor-producer-bin>" nomem)
