set(TARGET_NAME asapo-broker)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json settings_good.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/auth_secret.key auth_secret.key COPYONLY)
configure_file(settings_bad.json settings_bad.json COPYONLY)
add_script_test("${TARGET_NAME}-readconfig" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME>" nomem
        )
