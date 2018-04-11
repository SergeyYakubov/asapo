set(TARGET_NAME receiver)

################################
# Testing
################################
add_script_test("${TARGET_NAME}-monitoring" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin>" nomem
        )
