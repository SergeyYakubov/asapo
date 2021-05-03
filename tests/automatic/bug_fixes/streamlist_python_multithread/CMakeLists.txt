set(TARGET_NAME consumer_python_memleak)


prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS python-lib-consumer BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER python-lib-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS asapo_consumer BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER asapo_producer BINARY_DIR)
endif()

add_script_test("${TARGET_NAME}" "${PYTHON_LIBS} ${PYTHON_LIBS_PRODUCER} ${Python_EXECUTABLE}" nomem)

configure_file(magic_producer.py magic_producer.py COPYONLY)

