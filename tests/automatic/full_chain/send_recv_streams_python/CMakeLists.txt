set(TARGET_NAME send_recv_streams_python)

if (UNIX)
    get_target_property(PYTHON_LIBS_CONSUMER python-lib-consumer BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER python-lib-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS_CONSUMER asapo_consumer BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER asapo_producer BINARY_DIR)
endif()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/send_recv_streams.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}" "${Python_EXECUTABLE} ${PYTHON_LIBS_CONSUMER} ${PYTHON_LIBS_PRODUCER} ${TEST_SCRIPT}" nomem)
