set(TARGET_NAME getnext_python)

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-consumer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" "${PYTHON_LIBS} ${Python_EXECUTABLE}" nomem)

configure_file(getnext.py getnext.py COPYONLY)