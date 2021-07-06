set(TARGET_NAME consumer_python_memleak)


if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-consumer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" "${PYTHON_LIBS} ${Python_EXECUTABLE}" nomem)

configure_file(memleak.py memleak.py COPYONLY)

