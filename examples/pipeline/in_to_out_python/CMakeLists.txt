set(TARGET_NAME pipeline_inout_python)

prepare_asapo()

if (UNIX)
    get_target_property(PYTHON_LIBS_WORKER python-lib2 BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER python-lib2-producer BINARY_DIR)
else()
    get_target_property(PYTHON_LIBS_WORKER asapo_worker BINARY_DIR)
    get_target_property(PYTHON_LIBS_PRODUCER asapo_producer BINARY_DIR)
endif()

file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/in_to_out.py TEST_SCRIPT )

add_script_test("${TARGET_NAME}" "python ${PYTHON_LIBS_WORKER} ${PYTHON_LIBS_PRODUCER} ${TEST_SCRIPT} " nomem)

