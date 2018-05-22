set(TARGET_NAME receiver)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json receiver.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/discovery_settings.json discovery.json COPYONLY)
add_script_test("${TARGET_NAME}-monitoring" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin> $<TARGET_PROPERTY:asapo-discovery,EXENAME>" nomem
        )
