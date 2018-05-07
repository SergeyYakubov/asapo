set(TARGET_NAME asapo-broker)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json settings.json COPYONLY)
add_script_test("${TARGET_NAME}-getnext" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>" nomem
        )
