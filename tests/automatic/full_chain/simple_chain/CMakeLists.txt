set(TARGET_NAME full_chain_simple_chain)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json receiver.json COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/broker_settings.json broker.json COPYONLY)

add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin> $<TARGET_PROPERTY:asapo-broker,EXENAME> $<TARGET_FILE:getnext_broker>" nomem)
