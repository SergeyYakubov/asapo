set(TARGET_NAME file_transfer_rest_api)

################################
# Testing
################################
prepare_asapo()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/authorizer_settings.json.tpl.in authorizer.json.tpl @ONLY)
add_script_test("${TARGET_NAME}" "" nomem)
