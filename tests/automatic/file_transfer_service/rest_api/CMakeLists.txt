set(TARGET_NAME file_transfer_rest_api)

################################
# Testing
################################

prepare_asapo()

add_script_test("${TARGET_NAME}" "" nomem)
