set(TARGET_NAME hidra2-broker)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/settings/broker_settings.json settings_good.json COPYONLY)
configure_file(settings_bad.json settings_bad.json COPYONLY)
add_script_test("${TARGET_NAME}-readconfig" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>" nomem
        )
