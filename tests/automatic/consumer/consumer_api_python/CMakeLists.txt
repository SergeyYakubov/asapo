set(TARGET_NAME consumer_api_python)


prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-consumer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/authorizer_settings.json.tpl.in authorizer.json.tpl @ONLY)


add_script_test("${TARGET_NAME}" "${PYTHON_LIBS} ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}" nomem)

