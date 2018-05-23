set(TARGET_NAME receiver)

################################
# Testing
################################
get_target_property(RECEIVER_DIR receiver-bin BINARY_DIR)
get_target_property(RECEIVER_NAME receiver-bin OUTPUT_NAME)

configure_file(${CMAKE_SOURCE_DIR}/config/nomad/receiver.nmd.in  receiver.nmd)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json.tpl receiver.json.tpl COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/discovery_settings.json discovery.json COPYONLY)
add_script_test("${TARGET_NAME}-monitoring" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin> $<TARGET_PROPERTY:asapo-discovery,EXENAME>" nomem)
