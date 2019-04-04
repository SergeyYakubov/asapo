set(TARGET_NAME worker_api_python)


prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib2 BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_worker BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" ${PYTHON_LIBS} nomem)
configure_file(worker_api.py worker_api.py COPYONLY)

