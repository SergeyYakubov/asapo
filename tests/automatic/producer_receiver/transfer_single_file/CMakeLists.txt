set(TARGET_NAME transfer-single-file)

################################
# Testing
################################
configure_file(${CMAKE_SOURCE_DIR}/tests/automatic/settings/receiver.json receiver.json COPYONLY)
add_script_test("${TARGET_NAME}" "$<TARGET_FILE:dummy-data-producer> $<TARGET_FILE:receiver-bin>" nomem)
