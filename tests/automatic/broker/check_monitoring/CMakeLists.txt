set(TARGET_NAME asapo-broker)

################################
# Testing
################################
prepare_asapo()
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json settings.json COPYONLY)

add_script_test("${TARGET_NAME}-monitoring" "$<TARGET_PROPERTY:${TARGET_NAME},EXENAME> $<TARGET_PROPERTY:asapo,EXENAME>" nomem
        )
