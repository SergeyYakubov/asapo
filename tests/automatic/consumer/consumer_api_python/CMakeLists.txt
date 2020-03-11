set(TARGET_NAME consumer_api_python)


prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" "${PYTHON_LIBS} ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}" nomem)

