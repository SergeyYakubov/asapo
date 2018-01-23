set(TARGET_NAME hidra2-broker)

################################
# Testing
################################
add_script_test("${TARGET_NAME}-getnext" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>"
        )
