set(TARGET_NAME asapo-broker)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json settings.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/auth_secret.key auth_secret.key COPYONLY)

add_script_test("${TARGET_NAME}-getmeta" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME> $<TARGET_PROPERTY:asapo,EXENAME>" nomem
        )
