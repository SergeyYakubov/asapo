set(TARGET_NAME folder2db)

################################
# Testing
################################
add_script_test("${TARGET_NAME}-bin" "$<TARGET_FILE:${TARGET_NAME}-bin>"
        )
